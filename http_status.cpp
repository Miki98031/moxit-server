#include "http_status.h"

HttpStatus::HttpStatus(REQUEST_STATUS req_status, int client_fd)
    : m_client_fd(client_fd),
      m_read_header_bytes(0),
      m_read_body_bytes(0),
      m_sent_bytes(0),
      m_left_to_send(0),
      m_req_status(req_status)
{}
