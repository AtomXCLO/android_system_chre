#include <stdlib.h>
#include <array>
#include <chrono>
#include <fstream>
#include <thread>

#include <json/json.h>

#include <aidl/android/hardware/contexthub/BnContextHubCallback.h>
#include <aidl/android/hardware/contexthub/IContextHub.h>
#include <aidl/android/hardware/contexthub/NanoappBinary.h>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "hal_client_manager.h"

namespace android::hardware::contexthub::common::implementation {

namespace {
using aidl::android::hardware::contexthub::AsyncEventType;
using aidl::android::hardware::contexthub::BnContextHubCallback;
using aidl::android::hardware::contexthub::ContextHubMessage;
using aidl::android::hardware::contexthub::NanoappInfo;
using aidl::android::hardware::contexthub::NanSessionRequest;

using ndk::ScopedAStatus;

using ::testing::_;
using ::testing::ByMove;
using ::testing::IsEmpty;
using ::testing::Return;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

using HalClient = HalClientManager::HalClient;

static constexpr pid_t kSystemServerPid = 1000;
// The uuid assigned to ContextHubService
static const std::string kSystemServerUuid = "9a17008d6bf1445a90116d21bd985b6c";

static constexpr pid_t kVendorPid = 1001;
static const std::string kVendorUuid = "6e406b36cf4f4c0d8183db3708f45d8f";

const std::string kClientIdMappingFilePath = "./chre_hal_clients.json";

class ContextHubCallbackForTest : public BnContextHubCallback {
 public:
  ContextHubCallbackForTest(const std::string &uuid) {
    assert(uuid.length() == 32);  // 2 digits for one bytes x 16 bytes
    for (int i = 0; i < 16; i++) {
      mUuid[i] = strtol(uuid.substr(i * 2, 2).c_str(), /* end_ptr= */ nullptr,
                        /* base= */ 16);
    }
    ON_CALL(*this, handleContextHubAsyncEvent(_))
        .WillByDefault(Return(ByMove(ScopedAStatus::ok())));
  }
  ScopedAStatus handleNanoappInfo(
      const std::vector<NanoappInfo> & /*appInfo*/) override {
    return ScopedAStatus::ok();
  }

  ScopedAStatus handleContextHubMessage(
      const ContextHubMessage & /*message*/,
      const std::vector<std::string> & /*msgContentPerms*/) override {
    return ScopedAStatus::ok();
  }

  MOCK_METHOD(ScopedAStatus, handleContextHubAsyncEvent, (AsyncEventType event),
              (override));

  // Called after loading/unloading a nanoapp.
  ScopedAStatus handleTransactionResult(int32_t /*transactionId*/,
                                        bool /*success*/) override {
    return ScopedAStatus::ok();
  }

  ScopedAStatus handleNanSessionRequest(
      const NanSessionRequest & /* request */) override {
    return ScopedAStatus::ok();
  }
  ScopedAStatus getUuid(std::array<uint8_t, 16> *out_uuid) override {
    *out_uuid = mUuid;
    return ScopedAStatus::ok();
  }

 private:
  std::array<uint8_t, 16> mUuid{};
};

class HalClientManagerForTest : public HalClientManager {
 public:
  HalClientManagerForTest(DeadClientUnlinker deadClientUnlinker,
                          const std::string &clientIdMappingFilePath)
      : HalClientManager(std::move(deadClientUnlinker),
                         clientIdMappingFilePath){};

  const std::vector<HalClient> getClients() {
    return mClients;
  }

  static int64_t getTransactionTimeoutSeconds() {
    return kTransactionTimeoutThresholdMs / 1000;
  }

  static const char *getClientIdTag() {
    return kJsonClientId;
  }

  static const char *getUuidTag() {
    return kJsonUuid;
  }
};

class HalClientManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Clears out the mapping file content
    std::ofstream file(kClientIdMappingFilePath);
    ASSERT_TRUE(file.good());
  };
  void TearDown() override{};
};

auto mockDeadClientUnlinker =
    [](const std::shared_ptr<IContextHubCallback> & /*callback*/,
       void * /*deathRecipientCookie*/) { return true; };

std::unique_ptr<FragmentedLoadTransaction> createLoadTransaction(
    uint32_t transactionId) {
  uint64_t appId = 0x476f6f676cabcdef;
  uint32_t appVersion = 2;
  uint32_t appFlags = 3;
  uint32_t targetApiVersion = 4;
  std::vector<uint8_t> binary = {0xf0, 0xf1};
  return std::make_unique<FragmentedLoadTransaction>(
      transactionId, appId, appVersion, appFlags, targetApiVersion, binary,
      /* fragmentSize= */ 2048);
}

TEST_F(HalClientManagerTest, ClientIdMappingFile) {
  HalClientId systemClientId = 100;
  {
    // Write systemClientId into the mapping file
    Json::Value mappings;
    Json::Value mapping;
    mapping[HalClientManagerForTest::getClientIdTag()] = systemClientId;
    mapping[HalClientManagerForTest::getUuidTag()] = kSystemServerUuid;
    mappings.append(mapping);
    Json::StreamWriterBuilder factory;
    std::unique_ptr<Json::StreamWriter> const writer(factory.newStreamWriter());
    std::ofstream fileStream(kClientIdMappingFilePath);
    writer->write(mappings, &fileStream);
    fileStream << std::endl;
  }

  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> callback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  EXPECT_TRUE(halClientManager->registerCallback(kSystemServerPid, callback,
                                                 /* cookie= */ nullptr));

  std::vector<HalClient> clients = halClientManager->getClients();
  const HalClient &client = clients.front();
  EXPECT_THAT(clients, SizeIs(1));
  EXPECT_THAT(client.endpointIds, IsEmpty());
  EXPECT_EQ(client.callback, callback);
  EXPECT_EQ(client.uuid, kSystemServerUuid);
  EXPECT_EQ(client.pid, kSystemServerPid);
  // The client id allocated should be the one specified in the mapping file
  EXPECT_EQ(client.clientId, systemClientId);
}

TEST_F(HalClientManagerTest, CallbackRegistryBasic) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> callback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);

  EXPECT_TRUE(halClientManager->registerCallback(kSystemServerPid, callback,
                                                 /* cookie= */ nullptr));

  std::vector<HalClient> clients = halClientManager->getClients();
  const HalClient &client = clients.front();

  EXPECT_THAT(clients, SizeIs(1));
  EXPECT_THAT(client.endpointIds, IsEmpty());
  EXPECT_EQ(client.callback, callback);
  EXPECT_EQ(client.uuid, kSystemServerUuid);
  EXPECT_EQ(client.pid, kSystemServerPid);
  EXPECT_NE(client.clientId, kDefaultHalClientId);
}

TEST_F(HalClientManagerTest, CallbackRegistryTwiceFromSameClient) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> callbackA =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  std::shared_ptr<ContextHubCallbackForTest> callbackB =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);

  EXPECT_TRUE(halClientManager->registerCallback(kSystemServerPid, callbackA,
                                                 /* cookie= */ nullptr));
  EXPECT_THAT(halClientManager->getClients(), SizeIs(1));
  EXPECT_EQ(halClientManager->getClients().front().callback, callbackA);
  // Same client can override its callback
  EXPECT_TRUE(halClientManager->registerCallback(kSystemServerPid, callbackB,
                                                 /* cookie= */ nullptr));
  EXPECT_THAT(halClientManager->getClients(), SizeIs(1));
  EXPECT_EQ(halClientManager->getClients().front().callback, callbackB);
}

TEST_F(HalClientManagerTest, CallbackRetrievalByEndpoint) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> systemCallback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  std::shared_ptr<ContextHubCallbackForTest> vendorCallback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(kVendorUuid);
  uint16_t vendorEndpointId = 1;
  uint16_t systemServerEndpointId = 1;

  // Register the callbacks and endpoint ids
  EXPECT_TRUE(halClientManager->registerCallback(
      kSystemServerPid, systemCallback, /* deathRecipientCookie= */ nullptr));
  EXPECT_TRUE(halClientManager->registerEndpointId(kSystemServerPid,
                                                   systemServerEndpointId));
  EXPECT_TRUE(halClientManager->registerCallback(
      kVendorPid, vendorCallback, /* deathRecipientCookie= */ nullptr));
  EXPECT_TRUE(
      halClientManager->registerEndpointId(kVendorPid, vendorEndpointId));

  // Though endpoint ids have the same value, they should be mutated before
  // getting sent to CHRE and mapped to different callbacks
  EXPECT_TRUE(halClientManager->mutateEndpointIdFromHostIfNeeded(
      kVendorPid, vendorEndpointId));
  EXPECT_TRUE(halClientManager->mutateEndpointIdFromHostIfNeeded(
      kSystemServerPid, systemServerEndpointId));
  EXPECT_EQ(halClientManager->getCallbackForEndpoint(vendorEndpointId),
            vendorCallback);
  EXPECT_EQ(halClientManager->getCallbackForEndpoint(systemServerEndpointId),
            systemCallback);
}

TEST_F(HalClientManagerTest, TransactionRegistryAndOverridden) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> callback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  EXPECT_TRUE(halClientManager->registerCallback(
      kSystemServerPid, callback, /* deathRecipientCookie= */ nullptr));

  EXPECT_TRUE(halClientManager->registerPendingLoadTransaction(
      kSystemServerPid, createLoadTransaction(/* transactionId= */ 1)));

  // Immediate transaction overridden is not allowed as each transaction is
  // given a certain amount of time to finish
  EXPECT_FALSE(halClientManager->registerPendingLoadTransaction(
      kSystemServerPid, createLoadTransaction(/* transactionId= */ 2)));

  // Wait until the transaction is timed out to override it
  std::this_thread::sleep_for(std::chrono::seconds(
      HalClientManagerForTest::getTransactionTimeoutSeconds()));
  EXPECT_TRUE(halClientManager->registerPendingLoadTransaction(
      kSystemServerPid, createLoadTransaction(/* transactionId= */ 3)));
}

TEST_F(HalClientManagerTest, TransactionRegistryLoadAndUnload) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> callback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  EXPECT_TRUE(halClientManager->registerCallback(
      kSystemServerPid, callback, /* deathRecipientCookie= */ nullptr));

  EXPECT_TRUE(halClientManager->registerPendingUnloadTransaction(
      kSystemServerPid, /* transactionId= */ 1));

  // Load and unload transaction can't coexist because unloading a nanoapp that
  // is being loaded can cause problems.
  EXPECT_FALSE(halClientManager->registerPendingLoadTransaction(
      kSystemServerPid, createLoadTransaction(/* transactionId= */ 2)));

  // Clears out the pending unload transaction to register a new one.
  halClientManager->resetPendingUnloadTransaction(
      halClientManager->getClientId(kSystemServerPid), /* transactionId= */ 1);
  EXPECT_TRUE(halClientManager->registerPendingLoadTransaction(
      kSystemServerPid, createLoadTransaction(/* transactionId= */ 2)));
}

TEST_F(HalClientManagerTest, EndpointRegistry) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> systemCallback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  std::shared_ptr<ContextHubCallbackForTest> vendorCallback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(kVendorUuid);

  halClientManager->registerCallback(kSystemServerPid, systemCallback,
                                     /* cookie= */ nullptr);
  halClientManager->registerCallback(kVendorPid, vendorCallback,
                                     /* cookie= */ nullptr);

  std::vector<HalClient> clients = halClientManager->getClients();
  EXPECT_THAT(clients, SizeIs(2));
  // only system server can register endpoint ids > 63.

  EXPECT_TRUE(halClientManager->registerEndpointId(kSystemServerPid,
                                                   /* endpointId= */ 64));
  EXPECT_TRUE(halClientManager->registerEndpointId(kVendorPid,
                                                   /*endpointId= */ 63));
  EXPECT_FALSE(halClientManager->registerEndpointId(kVendorPid,
                                                    /* endpointId= */ 64));
}

TEST_F(HalClientManagerTest, EndpointIdMutationForVendorClient) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> vendorCallback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(kVendorUuid);
  std::shared_ptr<ContextHubCallbackForTest> systemCallback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  const uint16_t originalEndpointId = 10;  // 0b1010;
  uint16_t mutatedEndpointId = originalEndpointId;

  // Register the system callback
  EXPECT_TRUE(halClientManager->registerCallback(
      kSystemServerPid, systemCallback, /* deathRecipientCookie= */ nullptr));
  // Register the vendor callback
  EXPECT_TRUE(halClientManager->registerCallback(
      kVendorPid, vendorCallback, /* deathRecipientCookie= */ nullptr));

  // Mutate endpoint id from host to CHRE
  EXPECT_TRUE(halClientManager->mutateEndpointIdFromHostIfNeeded(
      kVendorPid, mutatedEndpointId));
  HalClientId clientId = halClientManager->getClientId(kVendorPid);
  EXPECT_EQ(mutatedEndpointId, 0x8000 | clientId << 6 | originalEndpointId);

  // Mutate endpoint id from CHRE to Host
  EXPECT_EQ(halClientManager->convertToOriginalEndpointId(mutatedEndpointId),
            originalEndpointId);
}

TEST_F(HalClientManagerTest, EndpointIdMutationForSystemServer) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> callback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  const uint16_t originalEndpointId = 100;
  uint16_t mutatedEndpointId = originalEndpointId;

  // Register the callback
  EXPECT_TRUE(halClientManager->registerCallback(
      kSystemServerPid, callback, /* deathRecipientCookie= */ nullptr));

  // Endpoint id from the system server shouldn't be mutated
  EXPECT_TRUE(halClientManager->mutateEndpointIdFromHostIfNeeded(
      kSystemServerPid, mutatedEndpointId));
  EXPECT_EQ(mutatedEndpointId, originalEndpointId);
  EXPECT_EQ(halClientManager->convertToOriginalEndpointId(mutatedEndpointId),
            originalEndpointId);
}

TEST_F(HalClientManagerTest, handleDeathClient) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> callback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  halClientManager->registerCallback(kSystemServerPid, callback,
                                     /* cookie= */ nullptr);
  halClientManager->registerEndpointId(kSystemServerPid, /* endpointId= */ 10);

  halClientManager->handleClientDeath(kSystemServerPid);

  const std::vector<HalClient> &clients = halClientManager->getClients();
  EXPECT_THAT(clients, SizeIs(1));
  const HalClient &client = clients.front();
  EXPECT_EQ(client.callback, nullptr);
  EXPECT_EQ(client.pid, HalClient::PID_UNSET);
  EXPECT_EQ(client.uuid, kSystemServerUuid);
  EXPECT_NE(client.clientId, kDefaultHalClientId);
  EXPECT_THAT(client.endpointIds, IsEmpty());
}

TEST_F(HalClientManagerTest, handleChreRestartForConnectedClientsOnly) {
  auto halClientManager = std::make_unique<HalClientManagerForTest>(
      mockDeadClientUnlinker, kClientIdMappingFilePath);
  std::shared_ptr<ContextHubCallbackForTest> vendorCallback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(kVendorUuid);
  std::shared_ptr<ContextHubCallbackForTest> systemCallback =
      ContextHubCallbackForTest::make<ContextHubCallbackForTest>(
          kSystemServerUuid);
  // Register the system callback
  EXPECT_TRUE(halClientManager->registerCallback(
      kSystemServerPid, systemCallback, /* deathRecipientCookie= */ nullptr));
  // Register the vendor callback
  EXPECT_TRUE(halClientManager->registerCallback(
      kVendorPid, vendorCallback, /* deathRecipientCookie= */ nullptr));

  // Only connected clients' handleContextHubAsyncEvent should be called.
  EXPECT_CALL(*systemCallback,
              handleContextHubAsyncEvent(AsyncEventType::RESTARTED));
  EXPECT_CALL(*vendorCallback,
              handleContextHubAsyncEvent(AsyncEventType::RESTARTED))
      .Times(0);

  // Disconnect the vendor client and handle CHRE restart for the system server
  halClientManager->handleClientDeath(kVendorPid);
  halClientManager->handleChreRestart();
}

}  // namespace
}  // namespace android::hardware::contexthub::common::implementation
