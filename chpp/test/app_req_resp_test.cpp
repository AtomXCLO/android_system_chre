/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <cstring>
#include <thread>

#include "chpp/app.h"
#include "chpp/clients.h"
#include "chpp/clients/discovery.h"
#include "chpp/macros.h"
#include "chpp/notifier.h"
#include "chpp/platform/platform_link.h"
#include "chpp/platform/utils.h"
#include "chpp/services.h"
#include "chpp/transport.h"
#include "chre/util/enum.h"
#include "chre/util/time.h"

namespace chre {
namespace {

constexpr uint64_t kResetWaitTimeMs = 1500;
constexpr uint64_t kDiscoveryWaitTimeMs = 5000;

void *workThread(void *transportState) {
  ChppTransportState *state = static_cast<ChppTransportState *>(transportState);

  auto linkContext =
      static_cast<struct ChppLinuxLinkState *>(state->linkContext);

  pthread_setname_np(pthread_self(), linkContext->workThreadName);

  chppWorkThreadStart(state);

  return nullptr;
}

#define TEST_UUID                                                           \
  {                                                                         \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x12                                              \
  }

enum class Commands : uint16_t {
  kOk,
  kError,
  kTimeout,
  // Number of request, must stay last
  kNumCommands,
};

constexpr uint16_t kNumCommands = asBaseType(Commands::kNumCommands);

// Common code for the client and the service.

struct CommonState {
  bool okResponseStatus;
  bool errorResponseStatus;
  bool timeoutResponseStatus;
  struct ChppNotifier notifier;
};

enum ChppAppErrorCode dispatchResponse(
    struct ChppAppState *appState,
    struct ChppOutgoingRequestState *outReqStates, struct CommonState *common,
    struct ChppAppHeader *response, size_t len) {
  // The response is composed of the app header only.
  if (len != sizeof(ChppAppHeader)) {
    return CHPP_APP_ERROR_NONE;
  }

  switch (response->command) {
    case asBaseType(Commands::kOk):
      // The response for the kOk command should have a CHPP_APP_ERROR_NONE
      // error.
      common->okResponseStatus = chppTimestampIncomingResponse(
          appState, &outReqStates[asBaseType(Commands::kOk)], response);

      common->okResponseStatus &= response->error == CHPP_APP_ERROR_NONE;
      return CHPP_APP_ERROR_NONE;

    case asBaseType(Commands::kError):
      // The response for the kError command should have a
      // CHPP_APP_ERROR_UNSPECIFIED error.
      common->errorResponseStatus = chppTimestampIncomingResponse(
          appState, &outReqStates[asBaseType(Commands::kError)], response);

      common->errorResponseStatus &=
          response->error == CHPP_APP_ERROR_UNSPECIFIED;
      return CHPP_APP_ERROR_NONE;

    case asBaseType(Commands::kTimeout):
      // The response for the kTimeout command should have a
      // CHPP_APP_ERROR_TIMEOUT error. That response is generated by the app
      // layer.
      common->timeoutResponseStatus = chppTimestampIncomingResponse(
          appState, &outReqStates[asBaseType(Commands::kTimeout)], response);

      common->timeoutResponseStatus &=
          response->error == CHPP_APP_ERROR_TIMEOUT;
      chppNotifierSignal(&common->notifier, 1 /*signal*/);
      return CHPP_APP_ERROR_NONE;

    default:
      return CHPP_APP_ERROR_NONE;
  }
}

enum ChppAppErrorCode dispatchRequest(
    struct ChppAppState *appState, struct ChppIncomingRequestState *inReqStates,
    struct ChppAppHeader *request, size_t len) {
  // The request is composed of the app header only.
  if (len != sizeof(ChppAppHeader)) {
    return CHPP_APP_ERROR_NONE;
  }

  switch (request->command) {
    case asBaseType(Commands::kOk): {
      // Return a response for the kOk command.
      chppTimestampIncomingRequest(&inReqStates[asBaseType(Commands::kOk)],
                                   request);

      struct ChppAppHeader *response =
          chppAllocResponse(request, sizeof(ChppAppHeader));

      chppSendTimestampedResponseOrFail(appState,
                                        &inReqStates[asBaseType(Commands::kOk)],
                                        response, sizeof(ChppAppHeader));
      return CHPP_APP_ERROR_NONE;
    }
    case asBaseType(Commands::kError): {
      // Return a response with a CHPP_APP_ERROR_UNSPECIFIED error on kError
      // command.
      chppTimestampIncomingRequest(&inReqStates[asBaseType(Commands::kError)],
                                   request);

      struct ChppAppHeader *response =
          chppAllocResponse(request, sizeof(ChppAppHeader));

      response->error = CHPP_APP_ERROR_UNSPECIFIED;

      chppSendTimestampedResponseOrFail(
          appState, &inReqStates[asBaseType(Commands::kError)], response,
          sizeof(ChppAppHeader));
      return CHPP_APP_ERROR_NONE;
    }

    case asBaseType(Commands::kTimeout): {
      // Do not send a response on kTimeout for the client to timeout.
      chppTimestampIncomingRequest(&inReqStates[asBaseType(Commands::kError)],
                                   request);

      return CHPP_APP_ERROR_NONE;
    }

    default:
      return CHPP_APP_ERROR_NONE;
  }
}

// Client specific code.
struct ClientState {
  struct ChppEndpointState chppClientState;
  struct ChppOutgoingRequestState outReqStates[kNumCommands];
  struct ChppIncomingRequestState inReqStates[kNumCommands];
  struct CommonState common;
};

bool clientInit(void *clientState, uint8_t handle,
                struct ChppVersion serviceVersion);
void clientDeinit(void *clientState);
enum ChppAppErrorCode clientDispatchResponse(void *clientState, uint8_t *buf,
                                             size_t len);
enum ChppAppErrorCode clientDispatchRequest(void *clientState, uint8_t *buf,
                                            size_t len);

constexpr struct ChppClient kClient = {
    .descriptor.uuid = TEST_UUID,
    .descriptor.version.major = 1,
    .descriptor.version.minor = 0,
    .descriptor.version.patch = 0,
    .resetNotifierFunctionPtr = nullptr,
    .matchNotifierFunctionPtr = nullptr,
    .responseDispatchFunctionPtr = &clientDispatchResponse,
    .notificationDispatchFunctionPtr = nullptr,
    .requestDispatchFunctionPtr = &clientDispatchRequest,
    .initFunctionPtr = &clientInit,
    .deinitFunctionPtr = &clientDeinit,
    .outReqCount = kNumCommands,
    .minLength = sizeof(struct ChppAppHeader),
};

// Called when a response is received from the service.
enum ChppAppErrorCode clientDispatchResponse(void *clientState, uint8_t *buf,
                                             size_t len) {
  CHPP_NOT_NULL(clientState);

  auto state = static_cast<struct ClientState *>(clientState);

  return dispatchResponse(state->chppClientState.appContext,
                          state->outReqStates, &state->common,
                          reinterpret_cast<struct ChppAppHeader *>(buf), len);
}

// Called when a request is received from the service.
enum ChppAppErrorCode clientDispatchRequest(void *clientState, uint8_t *buf,
                                            size_t len) {
  auto request = reinterpret_cast<struct ChppAppHeader *>(buf);
  auto state = static_cast<struct ClientState *>(clientState);

  return dispatchRequest(state->chppClientState.appContext, state->inReqStates,
                         request, len);
}

bool clientInit(void *clientState, uint8_t handle,
                struct ChppVersion serviceVersion) {
  UNUSED_VAR(serviceVersion);
  auto state = static_cast<struct ClientState *>(clientState);
  state->chppClientState.openState = CHPP_OPEN_STATE_OPENED;
  chppClientInit(&state->chppClientState, handle);
  return true;
}

void clientDeinit(void *clientState) {
  auto state = static_cast<struct ClientState *>(clientState);
  chppClientDeinit(&state->chppClientState);
  state->chppClientState.openState = CHPP_OPEN_STATE_CLOSED;
}

// Service specific code.

struct ServiceState {
  struct ChppEndpointState chppServiceState;
  struct ChppOutgoingRequestState outReqStates[kNumCommands];
  struct ChppIncomingRequestState inReqStates[kNumCommands];
  struct CommonState common;
};

// Called when a request is received from the client.
enum ChppAppErrorCode serviceDispatchRequest(void *serviceState, uint8_t *buf,
                                             size_t len) {
  auto request = reinterpret_cast<struct ChppAppHeader *>(buf);
  auto state = static_cast<struct ServiceState *>(serviceState);

  return dispatchRequest(state->chppServiceState.appContext, state->inReqStates,
                         request, len);
}

// Called when a response is received from the client.
enum ChppAppErrorCode serviceDispatchResponse(void *serviceState, uint8_t *buf,
                                              size_t len) {
  CHPP_NOT_NULL(serviceState);

  auto state = static_cast<struct ServiceState *>(serviceState);

  return dispatchResponse(state->chppServiceState.appContext,
                          state->outReqStates, &state->common,
                          reinterpret_cast<struct ChppAppHeader *>(buf), len);
}

const struct ChppService kService = {
    .descriptor.uuid = TEST_UUID,
    .descriptor.name = "Test",
    .descriptor.version.major = 1,
    .descriptor.version.minor = 0,
    .descriptor.version.patch = 0,
    .resetNotifierFunctionPtr = nullptr,
    .requestDispatchFunctionPtr = &serviceDispatchRequest,
    .notificationDispatchFunctionPtr = nullptr,
    .responseDispatchFunctionPtr = &serviceDispatchResponse,
    .outReqCount = kNumCommands,
    .minLength = sizeof(struct ChppAppHeader),
};

/**
 * Test requests and responses.
 *
 * The test parameter is:
 * - CHPP_MESSAGE_TYPE_CLIENT_REQUEST for client side requests
 * - CHPP_MESSAGE_TYPE_SERVICE_REQUEST for service side requests
 */
class AppReqRespParamTest : public testing::TestWithParam<ChppMessageType> {
 protected:
  void SetUp() {
    chppClearTotalAllocBytes();
    chppNotifierInit(&mClientState.common.notifier);
    chppNotifierInit(&mServiceState.common.notifier);
    memset(&mClientLinkContext, 0, sizeof(mClientLinkContext));
    memset(&mServiceLinkContext, 0, sizeof(mServiceLinkContext));

    mServiceLinkContext.linkThreadName = "Host Link";
    mServiceLinkContext.workThreadName = "Host worker";
    mServiceLinkContext.isLinkActive = true;
    mServiceLinkContext.remoteLinkState = &mClientLinkContext;
    mServiceLinkContext.rxInRemoteEndpointWorker = false;

    mClientLinkContext.linkThreadName = "CHRE Link";
    mClientLinkContext.workThreadName = "CHRE worker";
    mClientLinkContext.isLinkActive = true;
    mClientLinkContext.remoteLinkState = &mServiceLinkContext;
    mClientLinkContext.rxInRemoteEndpointWorker = false;

    // No default clients/services.
    struct ChppClientServiceSet set;
    memset(&set, 0, sizeof(set));

    const struct ChppLinkApi *linkApi = getLinuxLinkApi();

    // Init client side.
    chppTransportInit(&mClientTransportContext, &mClientAppContext,
                      &mClientLinkContext, linkApi);
    chppAppInitWithClientServiceSet(&mClientAppContext,
                                    &mClientTransportContext, set);

    // Init service side.
    chppTransportInit(&mServiceTransportContext, &mServiceAppContext,
                      &mServiceLinkContext, linkApi);
    chppAppInitWithClientServiceSet(&mServiceAppContext,
                                    &mServiceTransportContext, set);

    BringUpClient();
    std::this_thread::sleep_for(std::chrono::milliseconds(450));
    BringUpService();
    mClientLinkContext.linkEstablished = true;
    mServiceLinkContext.linkEstablished = true;

    chppTransportWaitForResetComplete(&mClientTransportContext,
                                      kResetWaitTimeMs);
    chppWaitForDiscoveryComplete(&mClientAppContext, kDiscoveryWaitTimeMs);
    chppWaitForDiscoveryComplete(&mServiceAppContext, kDiscoveryWaitTimeMs);
  }

  void BringUpClient() {
    memset(&mClientState, 0, sizeof(mClientState));
    chppRegisterClient(&mClientAppContext, &mClientState,
                       &mClientState.chppClientState,
                       &mClientState.outReqStates[0], &kClient);

    pthread_create(&mClientWorkThread, NULL, workThread,
                   &mClientTransportContext);
  }

  void BringUpService() {
    memset(&mServiceState, 0, sizeof(mServiceState));
    chppRegisterService(&mServiceAppContext, &mServiceState,
                        &mServiceState.chppServiceState,
                        &mServiceState.outReqStates[0], &kService);

    pthread_create(&mServiceWorkThread, NULL, workThread,
                   &mServiceTransportContext);
  }

  void TearDown() {
    chppNotifierDeinit(&mClientState.common.notifier);
    chppNotifierDeinit(&mServiceState.common.notifier);
    chppWorkThreadStop(&mClientTransportContext);
    chppWorkThreadStop(&mServiceTransportContext);
    pthread_join(mClientWorkThread, NULL);
    pthread_join(mServiceWorkThread, NULL);

    // Deinit client side.
    chppAppDeinit(&mClientAppContext);
    chppTransportDeinit(&mClientTransportContext);

    // Deinit service side.
    chppAppDeinit(&mServiceAppContext);
    chppTransportDeinit(&mServiceTransportContext);

    EXPECT_EQ(chppGetTotalAllocBytes(), 0);
  }

  struct ChppAppHeader *AllocRequestCommand(Commands command) {
    return GetParam() == CHPP_MESSAGE_TYPE_CLIENT_REQUEST
               ? chppAllocClientRequestCommand(&mClientState.chppClientState,
                                               asBaseType(command))
               : chppAllocServiceRequestCommand(&mServiceState.chppServiceState,
                                                asBaseType(command));
  }

  struct CommonState *GetCommonState() {
    return GetParam() == CHPP_MESSAGE_TYPE_CLIENT_REQUEST
               ? &mClientState.common
               : &mServiceState.common;
  }

  bool SendTimestampedRequestAndWait(struct ChppAppHeader *request) {
    constexpr size_t len = sizeof(struct ChppAppHeader);
    if (request->type == CHPP_MESSAGE_TYPE_CLIENT_REQUEST) {
      return chppClientSendTimestampedRequestAndWait(
          &mClientState.chppClientState,
          &mClientState.outReqStates[request->command], request, len);
    }

    return chppServiceSendTimestampedRequestAndWait(
        &mServiceState.chppServiceState,
        &mServiceState.outReqStates[request->command], request, len);
  }

  bool SendTimestampedRequestOrFail(struct ChppAppHeader *request,
                                    uint64_t timeoutNs) {
    constexpr size_t len = sizeof(struct ChppAppHeader);
    if (request->type == CHPP_MESSAGE_TYPE_CLIENT_REQUEST) {
      return chppClientSendTimestampedRequestOrFail(
          &mClientState.chppClientState,
          &mClientState.outReqStates[request->command], request, len,
          timeoutNs);
    }

    return chppServiceSendTimestampedRequestOrFail(
        &mServiceState.chppServiceState,
        &mServiceState.outReqStates[request->command], request, len, timeoutNs);
  }

  // Client side.
  ChppLinuxLinkState mClientLinkContext = {};
  ChppTransportState mClientTransportContext = {};
  ChppAppState mClientAppContext = {};
  pthread_t mClientWorkThread;
  ClientState mClientState;

  // Service side
  ChppLinuxLinkState mServiceLinkContext = {};
  ChppTransportState mServiceTransportContext = {};
  ChppAppState mServiceAppContext = {};
  pthread_t mServiceWorkThread;
  ServiceState mServiceState;
};

TEST_P(AppReqRespParamTest, sendsRequestAndReceiveResponse) {
  struct ChppAppHeader *request = AllocRequestCommand(Commands::kOk);
  ASSERT_NE(request, nullptr);

  GetCommonState()->okResponseStatus = false;

  EXPECT_TRUE(SendTimestampedRequestAndWait(request));

  EXPECT_TRUE(GetCommonState()->okResponseStatus);
}

TEST_P(AppReqRespParamTest, sendsRequestAndReceiveErrorResponse) {
  struct ChppAppHeader *request = AllocRequestCommand(Commands::kError);
  ASSERT_NE(request, nullptr);

  GetCommonState()->errorResponseStatus = false;

  EXPECT_TRUE(SendTimestampedRequestAndWait(request));

  EXPECT_TRUE(GetCommonState()->errorResponseStatus);
}

TEST_P(AppReqRespParamTest, sendsRequestAndReceiveTimeoutResponse) {
  struct ChppAppHeader *request = AllocRequestCommand(Commands::kTimeout);
  ASSERT_NE(request, nullptr);

  GetCommonState()->timeoutResponseStatus = false;

  EXPECT_TRUE(
      SendTimestampedRequestOrFail(request, 10 * kOneMicrosecondInNanoseconds));

  chppNotifierWait(&GetCommonState()->notifier);

  EXPECT_TRUE(GetCommonState()->timeoutResponseStatus);
}

INSTANTIATE_TEST_SUITE_P(
    AppReqRespTest, AppReqRespParamTest,
    testing::Values(CHPP_MESSAGE_TYPE_CLIENT_REQUEST,
                    CHPP_MESSAGE_TYPE_SERVICE_REQUEST),
    [](const testing::TestParamInfo<AppReqRespParamTest::ParamType> &info) {
      return info.param == CHPP_MESSAGE_TYPE_CLIENT_REQUEST ? "ClientRequests"
                                                            : "ServiceRequests";
    });

}  // namespace
}  // namespace chre