#include "handler.h"

Handler::Handler(Method method, const std::string &path, HandlerType handler)
    : m_method(method),
      m_path(path),
      m_handler(handler)
{}


bool Handler::is(Method method, const std::string &path) const {
    return method == m_method && std::regex_match(path, m_path);
}

void Handler::call(const Request &req, Response &res) const {
    m_handler(req, res);
}
