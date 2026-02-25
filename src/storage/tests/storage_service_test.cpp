#include "storage_service.hpp"
#include <gtest/gtest.h>

TEST(StorageServiceTest, CreateAndLoadEmptySnapshot) {
  auto service = storage::createStorageService("/nonexistent-path-12345");
  ASSERT_TRUE(service);

  auto snapshot = service->loadClusterSnapshot();
  ASSERT_TRUE(snapshot);
  EXPECT_TRUE(snapshot->topics_by_id.empty());
}

TEST(StorageServiceTest, FindTopicByNameEmptySnapshot) {
  auto service = storage::createStorageService("/tmp");
  auto snapshot = service->loadClusterSnapshot();
  ASSERT_TRUE(snapshot);

  auto topic = service->findTopicByName(*snapshot, "nonexistent");
  EXPECT_FALSE(topic.has_value());
}

TEST(StorageServiceTest, ReadPartitionDataMissingFile) {
  auto service = storage::createStorageService("/nonexistent-path-12345");
  ASSERT_TRUE(service);

  auto data = service->readPartitionData("nonexistent-topic", 0);
  ASSERT_TRUE(data);
  EXPECT_TRUE(data->empty());
}
