#ifndef MOXIT_SERVER_H
#define MOXIT_SERVER_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <vector>
#include <sys/epoll.h>
#include <functional>
#include <errno.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <signal.h>


#include "request.h"
#include "request_header.h"
#include "handler.h"
#include "http_status.h"

class MoxitServer {
    std::string m_ip_address;
    int m_port;
    int m_socket;

    struct sockaddr_in m_socketAddress;
    unsigned m_socketAddress_length;

    int startServer();
    void closeServer();

    int acceptConnection();
    void processConnection(int socket, HttpStatus **httpStatus) const;

private:
    bool m_running;
    int m_epfd;
    struct epoll_event m_evlist[1024];
    int m_numOfClients;
    std::vector<int> m_clientSocketFds;

    std::vector<std::thread> m_worker_threads;

    void setUpEpoll();
    void configureEpoll(int socket, epoll_event *event = nullptr);
    void addEpollWriteEvent(const Request& request, const std::string& response);

    void startWorkerThread();

    bool readFromClient(int client, HttpStatus **httpStatus);
    bool writeToClient(HttpStatus **httpStatus);

    static constexpr int k_backlogSize = 1024;
    static constexpr int k_threadPoolSize = 3;

public:
    MoxitServer(std::string ip_address, int port);
    ~MoxitServer();

    void startListen();

private:
    std::vector<Handler> m_handlers;

    void addHandlers();

public:
    MoxitServer& post(const std::string &path, const HandlerType &handler);
    MoxitServer& use(Method method, const std::string &path, const HandlerType &handler);
};

#endif // MOXIT_SERVER_H
