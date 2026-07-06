/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include <thread>

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "libs/httplib.h"
#include "HttpManager.h"
#include "LuaEngine.h"

HttpWorkItem::HttpWorkItem(sol::protected_function callback, const std::string& httpVerb, const std::string& url,
    const std::string& body, const std::string& contentType, const httplib::Headers& headers)
    : callback(std::move(callback)),
    httpVerb(httpVerb),
    url(url),
    body(body),
    contentType(contentType),
    headers(headers)
{ }

HttpResponse::HttpResponse(sol::protected_function callback, int statusCode, const std::string& body, const httplib::Headers& headers)
    : callback(std::move(callback)),
    statusCode(statusCode),
    body(body),
    headers(headers)
{ }

HttpManager::HttpManager()
    : workQueue(16),
    responseQueue(16),
    startedWorkerThread(false),
    cancelationToken(false),
    condVar(),
    condVarMutex(),
    parseUrlRegex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?")
{
    StartHttpWorker();
}

HttpManager::~HttpManager()
{
    StopHttpWorker();
}

void HttpManager::PushRequest(HttpWorkItem* item)
{
    std::unique_lock<std::mutex> lock(condVarMutex);
    workQueue.push(item);
    condVar.notify_one();
}

void HttpManager::StartHttpWorker()
{
    ClearQueues();

    if (!startedWorkerThread)
    {
        cancelationToken.store(false);
        workerThread = std::thread(&HttpManager::HttpWorkerThread, this);
        startedWorkerThread = true;
    }
}

void HttpManager::ClearQueues()
{
    while (workQueue.front())
    {
        HttpWorkItem* item = *workQueue.front();
        delete item;
        workQueue.pop();
    }

    while (responseQueue.front())
    {
        HttpResponse* item = *responseQueue.front();
        delete item;
        responseQueue.pop();
    }
}

void HttpManager::StopHttpWorker()
{
    if (!startedWorkerThread)
        return;

    cancelationToken.store(true);
    condVar.notify_one();
    workerThread.join();
    ClearQueues();
    startedWorkerThread = false;
}

void HttpManager::FinishRequest(HttpWorkItem* req, int statusCode, std::string body, httplib::Headers headers)
{
    // The callback must always come back to the world thread, even on
    // failure, so its Lua reference is released there and never here.
    responseQueue.push(new HttpResponse(std::move(req->callback), statusCode, std::move(body), std::move(headers)));
}

void HttpManager::HttpWorkerThread()
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(condVarMutex);
            condVar.wait(lock, [&] { return workQueue.front() != nullptr || cancelationToken.load(); });
        }

        if (cancelationToken.load())
            break;

        if (!workQueue.front())
            continue;

        HttpWorkItem* req = *workQueue.front();
        workQueue.pop();
        if (!req)
            continue;

        try
        {
            std::string host;
            std::string path;

            if (!ParseUrl(req->url, host, path))
            {
                ALE_LOG_ERROR("[ALE]: Could not parse URL {}", req->url);
                FinishRequest(req, -1, "", {});
                delete req;
                continue;
            }

            httplib::Client cli(host);
            cli.set_connection_timeout(0, 3000000); // 3 seconds
            cli.set_read_timeout(5, 0); // 5 seconds
            cli.set_write_timeout(5, 0); // 5 seconds

            httplib::Result res = DoRequest(cli, req, path);
            httplib::Error err = res.error();
            if (err != httplib::Error::Success)
            {
                ALE_LOG_ERROR("[ALE]: HTTP request error: {}", httplib::to_string(err));
                FinishRequest(req, -1, "", {});
                delete req;
                continue;
            }

            if (res->status == 301)
            {
                std::string location = res->get_header_value("Location");
                std::string redirectHost;
                std::string redirectPath;

                if (!ParseUrl(location, redirectHost, redirectPath))
                {
                    ALE_LOG_ERROR("[ALE]: Could not parse URL after redirect: {}", location);
                    FinishRequest(req, -1, "", {});
                    delete req;
                    continue;
                }
                httplib::Client cli2(redirectHost);
                cli2.set_connection_timeout(0, 3000000); // 3 seconds
                cli2.set_read_timeout(5, 0); // 5 seconds
                cli2.set_write_timeout(5, 0); // 5 seconds
                res = DoRequest(cli2, req, redirectPath);
            }

            FinishRequest(req, res->status, res->body, res->headers);
        }
        catch (const std::exception& ex)
        {
            ALE_LOG_ERROR("[ALE]: HTTP request error: {}", ex.what());
            FinishRequest(req, -1, "", {});
        }

        delete req;
    }
}

httplib::Result HttpManager::DoRequest(httplib::Client& client, HttpWorkItem* req, const std::string& urlPath)
{
    const char* path = urlPath.c_str();
    if (req->httpVerb == "GET")
        return client.Get(path, req->headers);
    if (req->httpVerb == "HEAD")
        return client.Head(path, req->headers);
    if (req->httpVerb == "POST")
        return client.Post(path, req->headers, req->body, req->contentType.c_str());
    if (req->httpVerb == "PUT")
        return client.Put(path, req->headers, req->body, req->contentType.c_str());
    if (req->httpVerb == "PATCH")
        return client.Patch(path, req->headers, req->body, req->contentType.c_str());
    if (req->httpVerb == "DELETE")
        return client.Delete(path, req->headers);
    if (req->httpVerb == "OPTIONS")
        return client.Options(path, req->headers);

    ALE_LOG_ERROR("[ALE]: HTTP request error: invalid HTTP verb {}", req->httpVerb);
    return client.Get(path, req->headers);
}

bool HttpManager::ParseUrl(const std::string& url, std::string& host, std::string& path)
{
    std::smatch matches;

    if (!std::regex_search(url, matches, parseUrlRegex))
        return false;

    std::string scheme = matches[2];
    std::string authority = matches[4];
    std::string query = matches[7];
    host = scheme + "://" + authority;
    path = matches[5];
    if (path.empty())
        path = "/";

    path += (query.empty() ? "" : "?") + query;

    return true;
}

void HttpManager::HandleHttpResponses()
{
    while (!responseQueue.empty())
    {
        HttpResponse* res = *responseQueue.front();
        responseQueue.pop();

        if (!res)
            continue;

        LOCK_ALE;

        // Failed requests only come back here to release their callback.
        if (res->statusCode >= 0 && sALE->HasLuaState())
        {
            // The handler receives the headers as a table.
            sol::table headerTable = sALE->lua.create_table();
            for (auto const& [name, value] : res->headers)
                headerTable[name] = value;

            sALE->CallFunction(res->callback, res->statusCode, res->body, headerTable);
        }

        delete res;
    }
}
