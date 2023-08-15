#ifndef HTTP_STATUS_H
#define HTTP_STATUS_H

#include <iostream>

using byte = unsigned char;
constexpr size_t MAX_BUFFER_SIZE = 3000;

enum class REQUEST_STATUS {
    Reading,
    Writing,
    Ended
};

using HttpStatus = struct HttpStatus {
    int m_client_fd;
    size_t m_read_header_bytes;
    size_t m_read_body_bytes;
    size_t m_sent_bytes;
    size_t m_left_to_send;
    REQUEST_STATUS m_req_status;
    char m_buffer[MAX_BUFFER_SIZE] = {};

public:
    HttpStatus(REQUEST_STATUS req_status, int client_fd = -1);
};
#endif // HTTP_STATUS_H
