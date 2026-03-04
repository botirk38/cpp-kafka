#include "../include/socket_fd.hpp"
#include <gtest/gtest.h>
#include <unistd.h>

TEST(SocketFdTest, CreateAndClose) {
  auto fd = SocketFd::create();
  EXPECT_TRUE(fd.valid());
  fd.close();
  EXPECT_FALSE(fd.valid());
}

TEST(SocketFdTest, MoveConstructor) {
  auto a = SocketFd::create();
  int raw = a.get();
  SocketFd b(std::move(a));
  EXPECT_FALSE(a.valid());
  EXPECT_TRUE(b.valid());
  EXPECT_EQ(b.get(), raw);
}

TEST(SocketFdTest, Release) {
  auto fd = SocketFd::create();
  int raw = fd.release();
  EXPECT_GE(raw, 0);
  EXPECT_FALSE(fd.valid());
  if (raw >= 0) {
    close(raw);
  }
}
