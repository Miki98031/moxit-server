#include "response.h"

Response::Response(int socket)
    : m_socket(socket),
      m_headers({{"connection", "close"},
                 {"content-type", "text-plain"},
                 {"server", "moxit_server.h"}
                }),
      m_body(),
      m_status(200)
{}

std::string Response::sendStatus(int status) {
    this->m_status = status;
    return send();
}

std::string Response::send(const std::string &body) {
    m_headers["content-length"] = std::to_string(body.length());

    std::stringstream buffer;
    buffer << "HTTP/1.1" << " " << m_status << " " << statusCodeMessage(m_status) << "\r\n";

    for(const auto &header: m_headers) {
        buffer << header.first << ": " << header.second << "\r\n";
    }
    buffer << "\r\n";

    buffer << body;
    std::string bufferStr = buffer.str();
    std::cout << bufferStr << std::endl;

    return bufferStr;
}

std::string Response::statusCodeMessage(int status) const {
    std::string message;
    switch(status) {
        case 100: message = "Continue"; break;
        case 101: message = "Switching Protocols"; break;
        case 102: message = "Processing"; break;
        case 103: message = "Early Hints"; break;
        case 200: message = "OK"; break;
        case 201: message = "Created"; break;
        case 202: message = "Accepted"; break;
        case 203: message = "Non-Authoritative Information"; break;
        case 205: message = "Reset Content"; break;
        case 206: message = "Partial Content"; break;
        case 207: message = "Multi-Status"; break;
        case 208: message = "Already Reported"; break;
        case 226: message = "IM Used"; break;
        case 300: message = "Multiple Choices"; break;
        case 301: message = "Moved Permanently"; break;
        case 302: message = "Found"; break;
        case 303: message = "See Other"; break;
        case 304: message = "Not Modified"; break;
        case 305: message = "Use Proxy"; break;
        case 307: message = "Temporary Redirect"; break;
        case 308: message = "Permanent Redirect"; break;
        case 400: message = "Bad Request"; break;
        case 401: message = "Unauthorized"; break;
        case 402: message = "Payment Required"; break;
        case 403: message = "Forbidden"; break;
        case 404: message = "Not Found"; break;
        case 405: message = "Method Not Allowed"; break;
        case 406: message = "Not Acceptable"; break;
        case 407: message = "Proxy Authentication Required"; break;
        case 408: message = "Request Timeout"; break;
        case 409: message = "Conflict"; break;
        case 410: message = "Gone"; break;
        case 411: message = "Length Required"; break;
        case 412: message = "Precondition Failed"; break;
        case 413: message = "Payload Too Large"; break;
        case 414: message = "URI Too Long"; break;
        case 415: message = "Unsupported Media Type"; break;
        case 416: message = "Range Not Satisfiable"; break;
        case 417: message = "Expectation Failed"; break;
        case 421: message = "Misdirected Request"; break;
        case 422: message = "Unprocessable Entity"; break;
        case 423: message = "Locked"; break;
        case 424: message = "Failed Dependency"; break;
        case 425: message = "Too Early"; break;
        case 426: message = "Upgrade Required"; break;
        case 428: message = "Precondition Required"; break;
        case 429: message = "Too Many Requests"; break;
        case 431: message = "Request Header Fields Too Large"; break;
        case 451: message = "Unavailable For Legal Reasons"; break;
        case 500: message = "Internal Server Error"; break;
        case 501: message = "Not Implemented"; break;
        case 502: message = "Bad Gateway"; break;
        case 503: message = "Service Unavailable"; break;
        case 504: message = "Gateway Timeout"; break;
        case 505: message = "HTTP Version Not Supported"; break;
        case 506: message = "Variant Also Negotiates"; break;
        case 507: message = "Insufficient Storage"; break;
        case 508: message = "Loop Detected"; break;
        case 509: message = "Bandwidth Limit Exceeded"; break;
        case 510: message = "Not Extended"; break;
        case 511: message = "Network Authentication Required"; break;

        default: message = "unknown status";
    }

    return message;
}
