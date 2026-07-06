/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef ALE_HTTP_MANAGER_H
#define ALE_HTTP_MANAGER_H

#include <sol/sol.hpp>

#include <regex>

#include "libs/httplib.h"
#include "libs/rigtorp/SPSCQueue.h"

/*
 * Threading note: the Lua callback travels through the worker thread as a
 * moved sol reference. Moving never touches the Lua state, so the worker
 * stays Lua-free; the callback is only invoked - and released - from the
 * world thread in HandleHttpResponses().
 */
struct HttpWorkItem
{
    HttpWorkItem(sol::protected_function callback, const std::string& httpVerb, const std::string& url,
        const std::string& body, const std::string& contentType, const httplib::Headers& headers);

    sol::protected_function callback;
    std::string httpVerb;
    std::string url;
    std::string body;
    std::string contentType;
    httplib::Headers headers;
};

struct HttpResponse
{
    HttpResponse(sol::protected_function callback, int statusCode, const std::string& body, const httplib::Headers& headers);

    sol::protected_function callback;
    // -1 when the request failed: the callback is not invoked, only released.
    int statusCode;
    std::string body;
    httplib::Headers headers;
};

class HttpManager
{
public:
    HttpManager();
    ~HttpManager();

    void StartHttpWorker();
    void StopHttpWorker();
    void PushRequest(HttpWorkItem* item);
    void HandleHttpResponses();

private:
    void ClearQueues();
    void HttpWorkerThread();
    // Hands the callback back to the world thread, with the request's result.
    void FinishRequest(HttpWorkItem* req, int statusCode, std::string body, httplib::Headers headers);
    bool ParseUrl(const std::string& url, std::string& host, std::string& path);
    httplib::Result DoRequest(httplib::Client& client, HttpWorkItem* req, const std::string& path);

    rigtorp::SPSCQueue<HttpWorkItem*> workQueue;
    rigtorp::SPSCQueue<HttpResponse*> responseQueue;
    std::thread workerThread;
    bool startedWorkerThread;
    std::atomic_bool cancelationToken;
    std::condition_variable condVar;
    std::mutex condVarMutex;
    std::regex parseUrlRegex;
};

#endif // ALE_HTTP_MANAGER_H
