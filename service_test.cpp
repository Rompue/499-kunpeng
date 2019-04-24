#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "cmd.h"

#include "service_data.pb.h"

class ServiceTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  CommandLineInterface service_ = CommandLineInterface(true);
  std::vector<std::string> usernametexts = {"username1", "username2",
                                            "username3", "username4"};
  std::vector<std::string> textset = {"sometext", "some long text",
                                      "some long long text",
                                      "some long long text"};
};

// register unexisted username should return true
TEST_F(ServiceTest, Register) {
  EXPECT_EQ(true, service_.isTesting());
  EXPECT_EQ(true, service_.registeruser("username"));
}

// repeated register should return false
TEST_F(ServiceTest, RegisterRepeated) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(true, service_.registeruser("username"));
  // register again with the same username
  EXPECT_EQ(false, service_.registeruser("username"));
}

// register empty username should return false
TEST_F(ServiceTest, RegisterEmptyUsername) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(false, service_.registeruser(""));
}

// login with a registered username should return true
TEST_F(ServiceTest, Login) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(true, service_.registeruser("username"));
  EXPECT_EQ(true, service_.login("username"));
}

// login with an unregistered username should return false
TEST_F(ServiceTest, LoginUnregistered) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(true, service_.registeruser("username"));
  EXPECT_EQ(false, service_.login("falseusername"));
}

// test post single chirp with `text`, should return true and relative fields of
// reply
TEST_F(ServiceTest, Post) {
  EXPECT_EQ(true, service_.isTesting());
  int index = -1;
  for (auto username : usernametexts) {
    index++;
    ASSERT_EQ(true, service_.registeruser(username));
    ASSERT_EQ(true, service_.login(username));
    ChirpReply reply;
    EXPECT_EQ(true, service_.postTest(textset[index], reply));
    EXPECT_EQ(std::to_string(index + 1), reply.chirp().id());
    EXPECT_EQ(username, reply.chirp().username());
    EXPECT_EQ(textset[index], reply.chirp().text());
  }
}

// test post chirp as reply
TEST_F(ServiceTest, PostReply) {
  EXPECT_EQ(true, service_.isTesting());
  int index = -1;
  for (auto username : usernametexts) {
    index++;
    ASSERT_EQ(true, service_.registeruser(username));
    ASSERT_EQ(true, service_.login(username));
    ChirpReply reply;
    if (index > 0) {  // post new chirp as a reply to the last chirp
      EXPECT_EQ(true, service_.postTest(textset[index], index, reply));
      EXPECT_EQ(std::to_string(index), reply.chirp().parent_id());
    } else {
      EXPECT_EQ(true, service_.postTest(textset[index], reply));
    }
    EXPECT_EQ(std::to_string(index + 1), reply.chirp().id());
    EXPECT_EQ(username, reply.chirp().username());
    EXPECT_EQ(textset[index], reply.chirp().text());
  }
}

// post chirp as a reply to an unexisted chirp should fail
TEST_F(ServiceTest, PostFail) {
  EXPECT_EQ(true, service_.isTesting());
  int index = -1;
  for (auto username : usernametexts) {
    index++;
    ASSERT_EQ(true, service_.registeruser(username));
    ASSERT_EQ(true, service_.login(username));
    ChirpReply reply;
    if (index > 0) {  // post new chirp as a reply to an unexited id
      EXPECT_EQ(false, service_.postTest(textset[index], index + 1, reply));
    } else {
      EXPECT_EQ(true, service_.postTest(textset[index], reply));
    }
  }
}

// read should read the chirp with given id into reply
TEST_F(ServiceTest, Read) {
  EXPECT_EQ(true, service_.isTesting());
  int index = -1;
  // post four chirps as reply to previous ones
  for (auto username : usernametexts) {
    index++;
    ASSERT_EQ(true, service_.registeruser(username));
    ASSERT_EQ(true, service_.login(username));
    ChirpReply reply;
    if (index > 0) {  // post new chirp as a reply to the last chirp
      EXPECT_EQ(true, service_.postTest(textset[index], index, reply));
      EXPECT_EQ(std::to_string(index), reply.chirp().parent_id());
    } else {
      EXPECT_EQ(true, service_.postTest(textset[index], reply));
    }
    EXPECT_EQ(std::to_string(index + 1), reply.chirp().id());
    EXPECT_EQ(username, reply.chirp().username());
    EXPECT_EQ(textset[index], reply.chirp().text());
  }
  ReadReply reply;
  EXPECT_EQ(true, service_.readTest(1, reply));
  // retrieve chirps
  ::google::protobuf::RepeatedPtrField< ::chirp::Chirp> chirps = reply.chirps();
  // check reply fields
  index = -1;
  for (auto username : usernametexts) {
    index++;
    if (index > 0) {  // new chirp as a reply to the last chirp
      EXPECT_EQ(std::to_string(index), chirps[index].parent_id());
    }
    EXPECT_EQ(std::to_string(index + 1), chirps[index].id());
    EXPECT_EQ(username, chirps[index].username());
    EXPECT_EQ(textset[index], chirps[index].text());
  }
}

// read should return false if given chirp id doesn't exist
TEST_F(ServiceTest, ReadFail) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(true, service_.registeruser("username"));
  EXPECT_EQ(true, service_.login("username"));
  ChirpReply chirpreply;
  EXPECT_EQ(true, service_.postTest("text", chirpreply));
  ReadReply readreply;
  EXPECT_EQ(true, service_.readTest(1, readreply));
  // read an unexisted chirp
  EXPECT_EQ(false, service_.readTest(2, readreply));
}

// follow a new user should add the username to followed list in storage
TEST_F(ServiceTest, Follow) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(true, service_.registeruser("followeduser"));
  ASSERT_EQ(true, service_.registeruser("username"));
  // login as `username`, follow `followeduser`
  EXPECT_EQ(true, service_.login("username"));
  EXPECT_EQ(true, service_.followuser("followeduser"));
  // check correct username stored in storage
  std::optional<std::vector<std::string> > usernames =
      service_.followedusernames();
  ASSERT_EQ(true, usernames.has_value());
  EXPECT_EQ(1, usernames.value().size());
  EXPECT_EQ("followeduser", usernames.value()[0]);

  // follow another user
  ASSERT_EQ(true, service_.registeruser("followeduser2"));
  EXPECT_EQ(true, service_.followuser("followeduser2"));
  // check correct username stored in storage in addition to previous one;
  usernames = service_.followedusernames();
  ASSERT_EQ(true, usernames.has_value());
  EXPECT_EQ(2, usernames.value().size());
  EXPECT_EQ("followeduser", usernames.value()[0]);
  EXPECT_EQ("followeduser2", usernames.value()[1]);
}

// follow an unexisted username should fail
TEST_F(ServiceTest, FollowFail) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(true, service_.registeruser("username"));
  // login as `username`, follow `followeduser` which does not exist, should
  // return false
  EXPECT_EQ(true, service_.login("username"));
  EXPECT_EQ(false, service_.followuser("followeduser"));
}

// test monitor behavior
TEST_F(ServiceTest, Monitor) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(true, service_.registeruser("username"));
  ASSERT_EQ(true, service_.login("username"));  // login as username
  ASSERT_EQ(true, service_.registeruser("followeduser"));
  ASSERT_EQ(true,
            service_.followuser("followeduser"));  // follow `followeduser`
  auto addedchirps = service_.monitorTest();       // start monitor
  // login as followeduser and post a chirp
  ASSERT_EQ(true, service_.login("followeduser"));
  ChirpReply reply;
  ASSERT_EQ(true, service_.postTest("this chirp should be monitored", reply));
  // login as username and check for monitor new chirp
  ASSERT_EQ(true, service_.login("username"));
  addedchirps = service_.monitorTest();
  ASSERT_EQ(1, addedchirps.size());
  // the monitored chirp should be the same with posted one
  EXPECT_EQ(reply.chirp().id(), addedchirps[0].id());
  EXPECT_EQ(reply.chirp().username(), addedchirps[0].username());
  EXPECT_EQ(reply.chirp().text(), addedchirps[0].text());
  EXPECT_EQ(reply.chirp().text(), addedchirps[0].text());
  EXPECT_EQ(reply.chirp().timestamp().seconds(),
            addedchirps[0].timestamp().seconds());
  EXPECT_EQ(reply.chirp().timestamp().useconds(),
            addedchirps[0].timestamp().useconds());
}

// test monitor behavior with multiple chirps posted
TEST_F(ServiceTest, MonitorMultiple) {
  EXPECT_EQ(true, service_.isTesting());
  for (auto username : usernametexts) {  // register 4 users
    ASSERT_EQ(true, service_.registeruser(username));
  }
  // login as first one and follow the rest 3
  ASSERT_EQ(true, service_.login(usernametexts[0]));
  for (auto username : usernametexts) {
    if (username == usernametexts[0]) continue;
    ASSERT_EQ(true, service_.followuser(username));
  }
  // start monitor
  auto addedchirps = service_.monitorTest();
  // login as followed 3 users and post a chirp each, record posted chirps
  std::vector<ChirpReply> replies;
  for (auto username : usernametexts) {
    if (username == usernametexts[0]) continue;
    ASSERT_EQ(true, service_.login(username));
    ChirpReply reply;
    ASSERT_EQ(true, service_.postTest("this chirp should be monitored", reply));
    replies.push_back(reply);
  }
  // login as first user and check for monitor new chirps
  ASSERT_EQ(true, service_.login(usernametexts[0]));
  addedchirps = service_.monitorTest();
  ASSERT_EQ(3, addedchirps.size());
  for (int i = 0; i < 3; i++) {
    // the monitored chirps should be the same with posted ones
    EXPECT_EQ(replies[i].chirp().id(), addedchirps[i].id());
    EXPECT_EQ(replies[i].chirp().username(), addedchirps[i].username());
    EXPECT_EQ(replies[i].chirp().text(), addedchirps[i].text());
    EXPECT_EQ(replies[i].chirp().text(), addedchirps[i].text());
    EXPECT_EQ(replies[i].chirp().timestamp().seconds(),
              addedchirps[i].timestamp().seconds());
    EXPECT_EQ(replies[i].chirp().timestamp().useconds(),
              addedchirps[i].timestamp().useconds());
  }
}

// test monitor should not output chirps posted before the monitor call
TEST_F(ServiceTest, MonitorBefore) {
  EXPECT_EQ(true, service_.isTesting());
  ASSERT_EQ(true, service_.registeruser("username"));
  ASSERT_EQ(true, service_.login("username"));  // login as username
  ASSERT_EQ(true, service_.registeruser("followeduser"));
  ASSERT_EQ(true,
            service_.followuser("followeduser"));  // follow `followeduser`
  // post a chirp as followed user before monitor as username
  ASSERT_EQ(true, service_.login("followeduser"));
  ChirpReply reply;
  ASSERT_EQ(true,
            service_.postTest("this chirp should not be monitored", reply));
  // login as username to start monitoring, should not monitor any chirps
  ASSERT_EQ(true, service_.login("username"));
  auto addedchirps = service_.monitorTest();  // start monitor
  EXPECT_EQ(0, addedchirps.size());
  // below tests posting a chirp after starting monitoring
  // login as followeduser and post a chirp
  ASSERT_EQ(true, service_.login("followeduser"));
  ASSERT_EQ(true, service_.postTest("this chirp should be monitored", reply));
  // login as username and check for monitor new chirp
  ASSERT_EQ(true, service_.login("username"));
  addedchirps = service_.monitorTest();
  EXPECT_EQ(1, addedchirps.size());
  // the monitored chirp should be the same with posted one
  EXPECT_EQ(reply.chirp().id(), addedchirps[0].id());
  EXPECT_EQ(reply.chirp().username(), addedchirps[0].username());
  EXPECT_EQ(reply.chirp().text(), addedchirps[0].text());
  EXPECT_EQ(reply.chirp().text(), addedchirps[0].text());
  EXPECT_EQ(reply.chirp().timestamp().seconds(),
            addedchirps[0].timestamp().seconds());
  EXPECT_EQ(reply.chirp().timestamp().useconds(),
            addedchirps[0].timestamp().useconds());
}

class ServiceTagTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // get the pointers of service and its storage client
    service_.reset(new ServiceImpl(true));
  }

  std::unique_ptr<ServiceImpl> service_;
};

TEST_F(ServiceTagTest, StoreTagToDatabase) {
  const std::string text = "abc #tag1 abc#tag2 abc#tag3#tag4\nabc #tag5";
  const std::vector<std::string> correct_tags = {"tag1", "tag2", "tag3#tag4",
                                                 "tag5"};

  Timestamp now_timestamp;
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count() /
             1000;
  now_timestamp.set_seconds(now);

  ServerContext context;
  ChirpRequest request;
  ChirpReply reply;

  request.set_text(text);

  service_->chirp(&context, &request, &reply);

  for (const std::string& tag : correct_tags) {
    auto ids = service_->GetChirpsByTagFromTime(tag, now_timestamp);
    ASSERT_TRUE(ids.chirp_ids_size() > 0);
    EXPECT_EQ(reply.chirp().id(), ids.chirp_ids(0));
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}