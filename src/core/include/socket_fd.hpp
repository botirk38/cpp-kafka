#pragma once

#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

class SocketFd {
public:
  SocketFd() = default;
  explicit SocketFd(int fd) : fd_(fd) {}

  SocketFd(const SocketFd &) = delete;
  SocketFd &operator=(const SocketFd &) = delete;

  SocketFd(SocketFd &&other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
  SocketFd &operator=(SocketFd &&other) noexcept {
    if (this != &other) {
      close();
      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }

  ~SocketFd() { close(); }

  void close() {
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
  }

  [[nodiscard]] int get() const { return fd_; }
  [[nodiscard]] bool valid() const { return fd_ >= 0; }

  int release() {
    int fd = fd_;
    fd_ = -1;
    return fd;
  }

  static SocketFd create() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
      throw std::system_error(errno, std::generic_category(), "Failed to create socket");
    }
    return SocketFd(fd);
  }

  void setReuseAddr() {
    int reuse = 1;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
      close();
      throw std::system_error(errno, std::generic_category(), "Failed to set socket options");
    }
  }

  void bind(const struct sockaddr_in &addr) {
    if (::bind(fd_, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr)) != 0) {
      close();
      throw std::system_error(errno, std::generic_category(), "Failed to bind");
    }
  }

  void listen(int backlog = 5) {
    if (::listen(fd_, backlog) != 0) {
      close();
      throw std::system_error(errno, std::generic_category(), "Failed to listen");
    }
  }

  SocketFd accept(struct sockaddr_in &client_addr, socklen_t &len) {
    int client_fd = ::accept(fd_, reinterpret_cast<struct sockaddr *>(&client_addr), &len);
    if (client_fd < 0) {
      return SocketFd{};
    }
    return SocketFd(client_fd);
  }

  void setSendTimeout(int seconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  }

private:
  int fd_{-1};
};
