#include "io/path_resolver.hpp"
#include <gtest/gtest.h>

using namespace storage::io;

TEST(PathResolverTest, ClusterMetadataPath) {
  PathResolver resolver("/var/log/kafka");
  EXPECT_EQ(resolver.clusterMetadataPath(),
            "/var/log/kafka/__cluster_metadata-0/00000000000000000000.log");
}

TEST(PathResolverTest, PartitionLogPath) {
  PathResolver resolver("/data");
  EXPECT_EQ(resolver.partitionLogPath("my-topic", 0), "/data/my-topic-0/00000000000000000000.log");
  EXPECT_EQ(resolver.partitionLogPath("test", 5), "/data/test-5/00000000000000000000.log");
}
