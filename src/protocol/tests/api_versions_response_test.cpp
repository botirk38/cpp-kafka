#include "../responses/include/api_version_response.hpp"
#include <cstring>
#include <gtest/gtest.h>

TEST(ApiVersionsResponseTest, WritesValidResponse) {
  char buf[256] = {};
  ApiVersionResponse writer(buf);
  writer.writeHeader(1, 0)
      .writeApiVersionSupport()
      .writeDescribeTopicsSupport()
      .writeFetchSupport()
      .writeMetadata()
      .complete();
  EXPECT_GT(writer.getOffset(), 0);
}

TEST(ApiVersionsResponseTest, UnsupportedVersion) {
  char buf[256] = {};
  ApiVersionResponse writer(buf);
  writer.writeHeader(1, 99).writeApiVersionSupport();
  EXPECT_EQ(writer.getOffset(), 4 + 4 + 2 + 1 + 2 + 2 + 2 + 1);
}
