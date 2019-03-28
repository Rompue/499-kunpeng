#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "storage_client.h"

class StorageTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  StorageClient client_ = StorageClient(true);
};

// `put` should store corresponding key value pair (also tests has)
TEST_F(StorageTest, PutOne) {
  EXPECT_EQ(true, client_.isTesting());
  client_.put("key", "value");
  std::optional<std::string> result = client_.has("key");
  ASSERT_EQ(true, result.has_value());
  EXPECT_EQ("value", result.value());
}

// `put` should update value if key exists
TEST_F(StorageTest, OverrideValue) {
  EXPECT_EQ(true, client_.isTesting());
  client_.put("key", "value");
  std::optional<std::string> result = client_.has("key");
  ASSERT_EQ(true, result.has_value());
  EXPECT_EQ("value", result.value());
  client_.put("key", "newvalue");
  result = client_.has("key");
  ASSERT_EQ(true, result.has_value());
  EXPECT_EQ("newvalue", result.value());
}

// `put` empty value should work
TEST_F(StorageTest, PutEmptyValue) {
  EXPECT_EQ(true, client_.isTesting());
  client_.put("key", "");
  std::optional<std::string> result = client_.has("key");
  ASSERT_EQ(true, result.has_value());
  EXPECT_EQ("", result.value());
}

// `put` empty key should work
TEST_F(StorageTest, PutEmptyKey) {
  EXPECT_EQ(true, client_.isTesting());
  client_.put("", "value");
  std::optional<std::string> result = client_.has("");
  ASSERT_EQ(true, result.has_value());
  EXPECT_EQ("value", result.value());
}

// `get` should retrieve the value under given key
TEST_F(StorageTest, GetOne) {
  EXPECT_EQ(true, client_.isTesting());
  client_.put("key", "value");
  std::string result = client_.get("key");
  EXPECT_EQ("value", result);
}

// `deletekey` should delete the key value pair
TEST_F(StorageTest, DeleteOneKey) {
  EXPECT_EQ(true, client_.isTesting());
  client_.put("key", "value");
  std::optional<std::string> result = client_.has("key");
  EXPECT_EQ(true, result.has_value());
  client_.deletekey("key");
  result = client_.has("key");
  EXPECT_EQ(false, result.has_value());
}

// should be able to call deletekey on nonexist key, nothing happens
TEST_F(StorageTest, DeleteNonexistKey) {
  EXPECT_EQ(true, client_.isTesting());
  client_.deletekey("nonexistkey");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}