#include "request_header.h"

RequestHeader::RequestHeader(Method method, std::string path,
              std::unordered_map<std::string, std::string> kv)
    : m_method(method),
      m_path(path),
      m_kv(kv)
{}

RequestHeader RequestHeader::parse(const std::string &headerStr) {
    Method method = Method::POST;
    std::string path;
    std::unordered_map<std::string, std::string> kv;

    auto lines = RequestHeader::split(headerStr, "\r\n");
    auto requestLine = RequestHeader::split(lines[0], " ");

    if(requestLine[0] == "post") {
        method = Method::POST;
    }

    path = requestLine[1];

    for(size_t i = 1 ; i < lines.size() ; i++) {
        auto keyValue = RequestHeader::split(lines[i], ": ");
        kv.insert(std::make_pair(keyValue[0], keyValue[1]));
    }

    RequestHeader rh(method, path, kv);

    return rh;

}

std::vector<std::string> RequestHeader::split(const std::string &headerStr,
                                              const std::string &delim) {
    std::vector<std::string> listStr;
    size_t strPos = 0;
    size_t delimPos = 0;

    if(delim == "") {
        listStr.push_back(headerStr);
    }

    else {
        while(true) {
            delimPos = headerStr.find(delim, strPos);
            if(delimPos == std::string::npos) {
                listStr.push_back(headerStr.substr(strPos));
                break;
            }

            listStr.push_back(headerStr.substr(strPos, delimPos - strPos));
            strPos = delimPos + delim.size();
        }
    }

    return listStr;
}

Method RequestHeader::getMethod() const {
    return this->m_method;
}

const std::string& RequestHeader::getPath() const {
    return this->m_path;
}
