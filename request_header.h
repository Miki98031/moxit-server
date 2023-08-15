#ifndef REQUESTHEADER_H
#define REQUESTHEADER_H

#include <iostream>
#include <unordered_map>
#include <vector>

enum class Method {
    POST
};

class RequestHeader {
    Method m_method;
    std::string m_path;
    std::unordered_map<std::string, std::string> m_kv;

public:
    RequestHeader(Method method, std::string path,
                  std::unordered_map<std::string, std::string> kv);

    static RequestHeader parse(const std::string &headerStr);

    Method getMethod() const;

    const std::string& getPath() const;

private:
    static std::vector<std::string> split(const std::string &headerStr,
                                          const std::string &delim);
};

#endif // REQUESTHEADER_H
