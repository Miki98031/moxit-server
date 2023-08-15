#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>

#include "request_header.h"

class Request {
    RequestHeader m_header;
    std::string m_body;
    int m_socket;
public:
    Request(RequestHeader header, const std::string &body, int socket);

    const std::string& getBody() const;

    const int getSocket() const;
};

#endif // REQUEST_H
