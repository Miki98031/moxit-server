#ifndef RESPONSE_H
#define RESPONSE_H

#include <iostream>
#include <unordered_map>
#include <string.h>
#include <sstream>
#include <sys/epoll.h>

class Response {
    int m_socket;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
    int m_status;
public:
    Response(int socket);

    std::string sendStatus(int status);

    std::string send(const std::string& body = "");

    std::string statusCodeMessage(int status) const;
};

#endif // RESPONSE_H
