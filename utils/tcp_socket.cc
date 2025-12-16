#include "./tcp_socket.hh"
#include <cstring>

constexpr int MAX_BACKLOG = 16;
constexpr int MAX_BUFSIZE = 1024;

TCPSocket::TCPSocket() 
    : 
    _fd(-1), 
    _ip(""), 
    _port(0) {
    _fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_fd < 0) {
        std::cerr << "socket: create error." << std::endl;
    }
}

TCPSocket::TCPSocket(const std::string &ip, uint16_t port) 
    : 
    _fd(-1), 
    _ip(ip), 
    _port(port) {
    _fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_fd < 0) {
        std::cerr << "socket: create error." << std::endl;
    }
}

TCPSocket::TCPSocket(int fd, const std::string &ip, uint16_t port) 
    : 
    _fd(fd), 
    _ip(ip), 
    _port(port) {}

TCPSocket::TCPSocket(const TCPSocket &sock) 
    :
    _fd(sock.fd()), 
    _ip(sock.ip()), 
    _port(sock.port()) {}

TCPSocket::TCPSocket(TCPSocket &sock) 
    : 
    _fd(sock.fd()), 
    _ip(sock.ip()), 
    _port(sock.port()) {}

TCPSocket::TCPSocket(TCPSocket&& sock) 
    :
    _fd(std::move(sock._fd)), 
    _ip(std::move(sock._ip)),
    _port(std::move(sock._port)) {}

TCPSocket::~TCPSocket() { close(_fd); }

// to bind a name to the socket, and make it reusable
// return 0 on success; otherwise, return -1
int TCPSocket::socket_bind() {
    int ret=-1;
    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "socket: setsockopt error." << std::endl;
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    if (_ip.empty()) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
	else {
        if (inet_pton(AF_INET, _ip.c_str(), &addr.sin_addr) <= 0) {
            std::cerr << "socket: invalid IP address." << std::endl;
            return -1;
        }
    }
    ret = bind(_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        std::cerr << "socket: bind error." << std::endl;
    }
    return ret;
}

// to mark the socket as ready to accept incoming connections
// please use MAX_BACKLOG as the backlog argument
// return 0 on success; otherwise, return -1
int TCPSocket::socket_listen() {
    int ret = listen(_fd, MAX_BACKLOG);
    if (ret < 0) {
        std::cerr << "socket: listen error." << std::endl;
    }
    return ret;
}

// try to connect remote socket through ``ip'' and ``port''
// return 0 on success; otherwise, return -1
int TCPSocket::socket_connect(const std::string &ip, uint16_t port) {
	int ret = -1;
	struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "socket: invalid IP address for connection." << std::endl;
        return -1;
    }
    ret = connect(_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == 0) {
        _ip = ip;
        _port = port;
    }
    else if (ret < 0) {
        std::cerr << "socket: connect error." << std::endl;
    }
    return ret;
}

// to accept an incoming connection
// return a pointer to TCPSocket as the new socket
TCPSocketPtr TCPSocket::socket_accept() {
	struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int connfd = accept(_fd, (struct sockaddr*)&client_addr, &client_len);
    std::string ip = "";
    uint16_t port = 0;
    if (connfd >= 0) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        ip = std::string(client_ip);
        port = ntohs(client_addr.sin_port);
    }
	else {
		std::cerr << "socket: accept error." << std::endl;
	}
    return std::make_shared<TCPSocket>(connfd, ip, port);
}

// to receive a message
// please use MAX_BUFSIZE as the buffer's size
// return the msg received on success; otherwise, return an empty string
std::string TCPSocket::socket_recv() {
    char buffer[MAX_BUFSIZE] = {0};
    int ret = recv(_fd, buffer, MAX_BUFSIZE - 1, 0);;
    std::string msg = "";
    if (ret < 0) {
        std::cerr << "socket: recv error." << std::endl;
    }
	else if (ret == 0) {
		std::cerr << "socket: connection closed by peer." << std::endl;
	}
	else {
        msg = buffer;
    }
    return msg;
}

// to send a message which is ``msg''
// return 0 on success; otherwise, return -1
int TCPSocket::socket_send(const std::string &msg) {
    int ret = send(_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
    if (ret < 0) {
        std::cerr << "socket: send error." << std::endl;
    }
    return ret;
}

// return the file descriptor
int TCPSocket::fd() const {
    return _fd;
}

// return a string indicating the ip
std::string TCPSocket::ip() const {
    return _ip;
}

// return a unsigned integer indicating the port
uint16_t TCPSocket::port() const {
    return _port;
}

int TCPSocket::socket_shutdown() {
    int ret = shutdown(_fd, SHUT_RDWR);
    return ret;
}

std::ostream& operator<<(std::ostream &output, const TCPSocket &sock) {
    output << sock.ip() << ":" << sock.port();
    return output;
}

bool TCPSocket::operator<(const TCPSocket &sock) const {
    return fd() < sock.fd();
}