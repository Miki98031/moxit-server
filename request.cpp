#include "request.h"

Request::Request(RequestHeader header, const std::string &body, int socket)
    : m_header(header),
      m_body(body),
      m_socket(socket)
{}

const std::string& Request::getBody() const {
    return m_body;
}

const int Request::getSocket() const {
    return this->m_socket;
}
