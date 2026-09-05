#ifndef ALE_HTTP_MANAGER_H
#define ALE_HTTP_MANAGER_H

#include <regex>

#include "libs/httplib.h"
#include "libs/rigtorp/SPSCQueue.h"

class ALE;

struct HttpWorkItem
{
public:
    HttpWorkItem(int funcRef, ALE* state, const std::string& httpVerb, const std::string& url, const std::string& body, const std::string &contentType, const httplib::Headers& headers);

    int funcRef;
    ALE* state;
    std::string httpVerb;
    std::string url;
    std::string body;
    std::string contentType;
    httplib::Headers headers;
};

struct HttpResponse
{
public:
    HttpResponse(int funcRef, ALE* state, int statusCode, const std::string& body, const httplib::Headers& headers);

    int funcRef;
    ALE* state;
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

#endif // #ifndef ALE_HTTP_MANAGER_H
