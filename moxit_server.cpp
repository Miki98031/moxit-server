#include "moxit_server.h"

void log(const std::string &message) {
    std::cout << message << std::endl;
}

void closeWithError(const std::string &errorMessage) {
    log("Error: " + errorMessage);
    exit(EXIT_FAILURE);
}

MoxitServer::MoxitServer(std::string ip_address, int port)
    : m_ip_address(ip_address),
      m_port(port),
      m_socket(),
      m_socketAddress(),
      m_socketAddress_length(sizeof(m_socketAddress)),
      m_running(false),
      m_epfd(),
      m_evlist(),
      m_numOfClients(0)
{   
    this->m_socketAddress.sin_family = AF_INET;
    this->m_socketAddress.sin_port = htons(port);
    this->m_socketAddress.sin_addr.s_addr = inet_addr(ip_address.c_str());

    this->startServer();
}

MoxitServer::~MoxitServer() {
    this->closeServer();
}

int MoxitServer::startServer() {
    this->m_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(this->m_socket < 0) {
        closeWithError("Cannot create socket.");
        return 1;
    }

    const int opt = 1;
    if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &opt, sizeof(opt)) < 0) {
        closeWithError("Cannot set reuse address.");
        return 1;
    }
    if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        closeWithError("Cannot set reuse port.");
        return 1;
    }

    if(bind(m_socket, (sockaddr*) &m_socketAddress, m_socketAddress_length) < 0) {
        closeWithError("Cannot bind socket.");
        return 1;
    }

    return 0;
}

void MoxitServer::closeServer() {
    m_running = false;

    for(int i = 0 ; i < k_threadPoolSize ; i++) {
        //m_worker_threads[i].join();
    }

    if(close(m_epfd) < 0) {
        closeWithError("Close epfd.");
    }

    for(int i = 0 ; i < m_numOfClients ; i++) {
        close(m_clientSocketFds[i]);
    }

    close(this->m_socket);

    exit(EXIT_SUCCESS);
}

void MoxitServer::startListen() {
    addHandlers();

    if(listen(m_socket, k_backlogSize) < 0) {
        closeWithError("Cannot listen.");
        exit(EXIT_FAILURE);
    }

    std::ostringstream ss;

    ss << "\n*** Listening on ADDRESS: "
            << inet_ntoa(m_socketAddress.sin_addr)
            << " PORT: " << ntohs(m_socketAddress.sin_port)
            << " ***\n\n";

    log(ss.str());

    setUpEpoll();

    m_running = true;

    for(int i = 0 ; i < k_threadPoolSize ; i++) {
        //m_worker_threads.push_back(std::thread(&MoxitServer::startWorkerThread, this));
    }

    startWorkerThread();
}

void MoxitServer::setUpEpoll() {
    m_epfd = epoll_create(1024);
    if(m_epfd < 0) {
        closeWithError("Epoll_create error.");
    }

    struct epoll_event event{};
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_socket;

    if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_socket, &event) < 0) {
        closeWithError("Epoll_ctl error.");
    }
}

int MoxitServer::acceptConnection() {
    sockaddr_in client = {};
    socklen_t len = sizeof(client);

    int new_socket = accept4(m_socket, (sockaddr*) &client, &len, SOCK_NONBLOCK);
    if(new_socket < 0) {
        std::ostringstream ss;

        ss << "Server failed to accept incoming connection from ADDRESS: "
        << inet_ntoa(m_socketAddress.sin_addr) << "; PORT: "
        << ntohs(m_socketAddress.sin_port);

        closeWithError(ss.str());
    }

    struct epoll_event event{};

    HttpStatus httpStatus(REQUEST_STATUS::Reading, new_socket);

    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    event.data.ptr = static_cast<void*>(&httpStatus);

    if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, new_socket, &event) < 0) {
        closeWithError("Epoll_ctl error.");
    }

    return new_socket;
}


void MoxitServer::startWorkerThread() {
    while(m_running) {
        int numOfFds = epoll_wait(m_epfd, m_evlist, 1024, 2000);
        if(numOfFds < 0) {
            closeWithError("Epoll_wait error.");
        }

        for(int i = 0 ; i < numOfFds ; i++) {
            if(m_evlist[i].data.fd == m_socket) {
                int new_client = acceptConnection();
                std::cout << new_client << std::endl;
                m_numOfClients++;
                m_clientSocketFds.push_back(new_client);
            }
            else {
                HttpStatus *httpStatus = reinterpret_cast<HttpStatus*>(m_evlist[i].data.ptr);
                int client = httpStatus->m_client_fd;

                if(httpStatus->m_req_status == REQUEST_STATUS::Reading) {
                    bool readingFinished = readFromClient(client, &httpStatus);
                    //std::cout << httpStatus->m_buffer << std::endl;

                    if(readingFinished) {
                        processConnection(client, &httpStatus);
                    }

                    else {
                        m_evlist[i].events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                        m_evlist[i].data.ptr = static_cast<void*>(httpStatus);

                        if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, httpStatus->m_client_fd, &m_evlist[i]) < 0) {
                            closeWithError("Epoll_ctl_del error.");
                        }
                    }
                }

                else if(httpStatus->m_req_status == REQUEST_STATUS::Writing) {
                    bool writingFinished = writeToClient(&httpStatus);

                    if(writingFinished) {
                        httpStatus->m_req_status = REQUEST_STATUS::Ended;
                        httpStatus->m_buffer[0] = '\0';
                        if(close(httpStatus->m_client_fd) < 0) {
                            closeWithError("close");
                        }

                        //if(epoll_ctl(m_epfd, EPOLL_CTL_DEL, httpStatus->m_client_fd, &m_evlist[i]) < 0) {
                          //  closeWithError("Epoll_ctl_del error.");
                        //}
                    }
                    else {
                        m_evlist[i].events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
                        m_evlist[i].data.ptr = static_cast<void*>(httpStatus);

                        if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, httpStatus->m_client_fd, &m_evlist[i]) < 0) {
                            closeWithError("Epoll_ctl error.");
                        }
                    }
                }
            }

        }
    }
}

bool MoxitServer::readFromClient(int client, HttpStatus **httpStatus) {
    std::string bufferStr;
    bufferStr.append((*httpStatus)->m_buffer, (*httpStatus)->m_read_body_bytes);

    int i = 0;
    while(i < 4) {
        i++;
        char buffer[1024] = {};
        ssize_t readBytes = recv(client, buffer, 50, 0);

        if(readBytes > 0) {
            bufferStr.append(buffer, readBytes);
            (*httpStatus)->m_read_body_bytes += readBytes;
        }

        else if(readBytes == 0) { //Client closed connection

        }

        else { //readBytes < 0
            if(errno == EAGAIN || errno == EWOULDBLOCK) { //Source temporary unavailable
                auto emptyLinePos = bufferStr.find("\r\n\r\n");
                if(emptyLinePos != std::string::npos) {
                    (*httpStatus)->m_read_header_bytes = emptyLinePos;
                    (*httpStatus)->m_read_body_bytes -= ((*httpStatus)->m_read_header_bytes + 4);

                    strncpy((*httpStatus)->m_buffer, bufferStr.c_str(), 1024);

                    return true;
                }

                break;
            }

            else {
                closeWithError("recv error");
                break;
            }
        }
    }

    strncpy((*httpStatus)->m_buffer, bufferStr.c_str(), 1024);

    return false;
}

bool MoxitServer::writeToClient(HttpStatus **httpStatus) {
    size_t MAX_BYTES_TO_SEND = 50;
    bool ind = false;

    int i = 0;
    while(i < 4) {
        i++;
        if(ind) {
            return true;
        }

        if((*httpStatus)->m_left_to_send < MAX_BYTES_TO_SEND) {
            MAX_BYTES_TO_SEND = (*httpStatus)->m_left_to_send;
            ind = true;
        }
        ssize_t sendBytes = send((*httpStatus)->m_client_fd,
                                 (*httpStatus)->m_buffer + (*httpStatus)->m_sent_bytes,
                                 MAX_BYTES_TO_SEND, 0);

        if(sendBytes > 0) {
            (*httpStatus)->m_sent_bytes += sendBytes;
            (*httpStatus)->m_left_to_send -= sendBytes;
        }

        else if(sendBytes == 0) { //Client closed connection
            break;
        }

        else { //sendBytes < 0
            if(errno == EAGAIN || errno == EWOULDBLOCK) { //Source temporary unavailable
                break;
            }

            else {
                closeWithError("send error");
                break;
            }
        }
    }

    return false;
}

void MoxitServer::processConnection(int socket, HttpStatus **httpStatus) const {
    std::string headerStr;
    std::string bodyStr;

    headerStr.append((*httpStatus)->m_buffer, (*httpStatus)->m_read_header_bytes);
    bodyStr.append((*httpStatus)->m_buffer + (*httpStatus)->m_read_header_bytes + 4,
                   (*httpStatus)->m_read_body_bytes);

    auto reqHeader = RequestHeader::parse(headerStr);

    Response res = Response(socket);

    bool ind = false;
    for(const auto &handler: m_handlers) {
        if(handler.is(reqHeader.getMethod(), reqHeader.getPath())) {
            handler.call(Request(reqHeader, bodyStr, socket), res);
            ind = true;
            break;
        }
    }

    if(!ind) {
        std::string response = res.sendStatus(404);

        struct epoll_event event;

        HttpStatus httpStatus(REQUEST_STATUS::Writing, socket);
        event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;

        httpStatus.m_left_to_send = response.size();

        strncpy(httpStatus.m_buffer, response.c_str(), response.size());
        event.data.ptr = static_cast<void*>(&httpStatus);

        if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, socket, &event) < 0) {
                closeWithError("Epoll_ctl error.");
            }
    }
}

MoxitServer& MoxitServer::post(const std::string &path, const HandlerType &handler) {
    return this->use(Method::POST, path, handler);
}

MoxitServer& MoxitServer::use(Method method, const std::string &path, const HandlerType &handler) {
    Handler h = Handler(method, path, handler);
    m_handlers.emplace_back(h);
    return *this;
}

void MoxitServer::addEpollWriteEvent(const Request& request, const std::string& response) {
    struct epoll_event event;
    int socket = request.getSocket();

        HttpStatus httpStatus(REQUEST_STATUS::Writing, socket);
        event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;

        httpStatus.m_left_to_send = response.size();

        strncpy(httpStatus.m_buffer, response.c_str(), response.size());
        event.data.ptr = static_cast<void*>(&httpStatus);

        if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, socket, &event) < 0) {
            closeWithError("Epoll_ctl error.");
        }
}

void MoxitServer::addHandlers() {
    this->post("/", [this] (const Request &req, Response res) {
        std::cout << "Welcome." << std::endl;

        std::string response = res.send(req.getBody());
        this->addEpollWriteEvent(req, response);
    });

    this->post("/init", [this] (const Request &req, Response res) {
        std::cout << "Init function called." << std::endl;

        std::string response = res.send(req.getBody());
        this->addEpollWriteEvent(req, response);
    });

    this->post("/settings", [this] (const Request &req, Response res) {
        std::cout << "Settings function called." << std::endl;

        std::string response = res.send(req.getBody());
        this->addEpollWriteEvent(req, response);
    });

    this->post("/gameState", [this] (const Request &req, Response res) {
        std::cout << "GameState function called." << std::endl;

        std::string response = res.send(req.getBody());
        this->addEpollWriteEvent(req, response);
    });

    this->post("/chat", [this] (const Request &req, Response res) {
        std::cout << "Chat function called." << std::endl;

        std::string response = res.send(req.getBody());
        this->addEpollWriteEvent(req, response);
    });
}
