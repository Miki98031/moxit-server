#ifndef HANDLER_H
#define HANDLER_H

#include <iostream>
#include <functional>
#include <regex>

#include "request_header.h"
#include "request.h"
#include "response.h"

using HandlerType = std::function<void(const Request&, Response&)>;

class Handler {
    Method m_method;
    std::regex m_path;
    HandlerType m_handler;
public:
    Handler(Method method, const std::string &path, HandlerType handler);

    bool is(Method method, const std::string &path) const;
    void call(const Request &req, Response &res) const;
};

#endif // HANDLER_H
