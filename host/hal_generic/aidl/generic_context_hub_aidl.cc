/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "generic_context_hub_aidl.h"

#include "chre_api/chre/event.h"
#include "chre_host/config_util.h"
#include "chre_host/file_stream.h"
#include "chre_host/fragmented_load_transaction.h"
#include "chre_host/host_protocol_host.h"
#include "chre_host/napp_header.h"
#include "permissions_util.h"

namespace aidl::android::hardware::contexthub {

// Aliased for consistency with the way these symbols are referenced in
// CHRE-side code
namespace fbs = ::chre::fbs;

using ::android::chre::FragmentedLoadTransaction;
using ::android::chre::getPreloadedNanoappsFromConfigFile;
using ::android::chre::getStringFromByteVector;
using ::android::chre::NanoAppBinaryHeader;
using ::android::chre::readFileContents;
using ::android::hardware::contexthub::common::implementation::
    chreToAndroidPermissions;
using ::android::hardware::contexthub::common::implementation::
    kSupportedPermissions;
using ::ndk::ScopedAStatus;

namespace {
constexpr uint32_t kDefaultHubId = 0;
constexpr char kPreloadedNanoappsConfigPath[] =
    "/vendor/etc/chre/preloaded_nanoapps.json";

inline constexpr int8_t extractChreApiMajorVersion(uint32_t chreVersion) {
  return static_cast<int8_t>(chreVersion >> 24);
}

inline constexpr int8_t extractChreApiMinorVersion(uint32_t chreVersion) {
  return static_cast<int8_t>(chreVersion >> 16);
}

inline constexpr uint16_t extractChrePatchVersion(uint32_t chreVersion) {
  return static_cast<uint16_t>(chreVersion);
}

bool getFbsSetting(const Setting &setting, fbs::Setting *fbsSetting) {
  bool foundSetting = true;

  switch (setting) {
    case Setting::LOCATION:
      *fbsSetting = fbs::Setting::LOCATION;
      break;
    case Setting::AIRPLANE_MODE:
      *fbsSetting = fbs::Setting::AIRPLANE_MODE;
      break;
    case Setting::MICROPHONE:
      *fbsSetting = fbs::Setting::MICROPHONE;
      break;
    default:
      foundSetting = false;
      ALOGE("Setting update with invalid enum value %hhu", setting);
      break;
  }

  return foundSetting;
}

ScopedAStatus toServiceSpecificError(bool success) {
  return success ? ScopedAStatus::ok()
                 : ScopedAStatus::fromServiceSpecificError(
                       BnContextHub::EX_CONTEXT_HUB_UNSPECIFIED);
}

}  // anonymous namespace

ScopedAStatus ContextHub::getContextHubs(
    std::vector<ContextHubInfo> *out_contextHubInfos) {
  ::chre::fbs::HubInfoResponseT response;
  bool success = mConnection.getContextHubs(&response);
  if (success) {
    ContextHubInfo hub;
    hub.name = getStringFromByteVector(response.name);
    hub.vendor = getStringFromByteVector(response.vendor);
    hub.toolchain = getStringFromByteVector(response.toolchain);
    hub.id = kDefaultHubId;
    hub.peakMips = response.peak_mips;
    hub.maxSupportedMessageLengthBytes = response.max_msg_len;
    hub.chrePlatformId = response.platform_id;

    uint32_t version = response.chre_platform_version;
    hub.chreApiMajorVersion = extractChreApiMajorVersion(version);
    hub.chreApiMinorVersion = extractChreApiMinorVersion(version);
    hub.chrePatchVersion = extractChrePatchVersion(version);

    hub.supportedPermissions = kSupportedPermissions;

    out_contextHubInfos->push_back(hub);
  }

  return ndk::ScopedAStatus::ok();
}

ScopedAStatus ContextHub::loadNanoapp(int32_t contextHubId,
                                      const NanoappBinary &appBinary,
                                      int32_t transactionId) {
  if (contextHubId != kDefaultHubId) {
    ALOGE("Invalid ID %" PRId32, contextHubId);
    return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
  }
  uint32_t targetApiVersion = (appBinary.targetChreApiMajorVersion << 24) |
                              (appBinary.targetChreApiMinorVersion << 16);
  FragmentedLoadTransaction transaction(
      transactionId, appBinary.nanoappId, appBinary.nanoappVersion,
      appBinary.flags, targetApiVersion, appBinary.customBinary);
  const bool success = mConnection.loadNanoapp(transaction);
  mEventLogger.logNanoappLoad(appBinary, success);
  return toServiceSpecificError(success);
}

ScopedAStatus ContextHub::unloadNanoapp(int32_t contextHubId, int64_t appId,
                                        int32_t transactionId) {
  if (contextHubId != kDefaultHubId) {
    ALOGE("Invalid ID %" PRId32, contextHubId);
    return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
  }
  const bool success = mConnection.unloadNanoapp(appId, transactionId);
  mEventLogger.logNanoappUnload(appId, success);
  return toServiceSpecificError(success);
}

ScopedAStatus ContextHub::disableNanoapp(int32_t /* contextHubId */,
                                         int64_t appId,
                                         int32_t /* transactionId */) {
  ALOGW("Attempted to disable app ID 0x%016" PRIx64 ", but not supported",
        appId);
  return ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ScopedAStatus ContextHub::enableNanoapp(int32_t /* contextHubId */,
                                        int64_t appId,
                                        int32_t /* transactionId */) {
  ALOGW("Attempted to enable app ID 0x%016" PRIx64 ", but not supported",
        appId);
  return ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ScopedAStatus ContextHub::onSettingChanged(Setting setting, bool enabled) {
  mSettingEnabled[setting] = enabled;
  fbs::Setting fbsSetting;
  bool isWifiOrBtSetting =
      (setting == Setting::WIFI_MAIN || setting == Setting::WIFI_SCANNING ||
       setting == Setting::BT_MAIN || setting == Setting::BT_SCANNING);

  if (!isWifiOrBtSetting && getFbsSetting(setting, &fbsSetting)) {
    mConnection.sendSettingChangedNotification(fbsSetting,
                                               toFbsSettingState(enabled));
  }

  bool isWifiMainEnabled = isSettingEnabled(Setting::WIFI_MAIN);
  bool isWifiScanEnabled = isSettingEnabled(Setting::WIFI_SCANNING);
  bool isAirplaneModeEnabled = isSettingEnabled(Setting::AIRPLANE_MODE);

  // Because the airplane mode impact on WiFi is not standardized in Android,
  // we write a specific handling in the Context Hub HAL to inform CHRE.
  // The following definition is a default one, and can be adjusted
  // appropriately if necessary.
  bool isWifiAvailable = isAirplaneModeEnabled
                             ? (isWifiMainEnabled)
                             : (isWifiMainEnabled || isWifiScanEnabled);
  if (!mIsWifiAvailable.has_value() || (isWifiAvailable != mIsWifiAvailable)) {
    mConnection.sendSettingChangedNotification(
        fbs::Setting::WIFI_AVAILABLE, toFbsSettingState(isWifiAvailable));
    mIsWifiAvailable = isWifiAvailable;
  }

  // The BT switches determine whether we can BLE scan which is why things are
  // mapped like this into CHRE.
  bool isBtMainEnabled = isSettingEnabled(Setting::BT_MAIN);
  bool isBtScanEnabled = isSettingEnabled(Setting::BT_SCANNING);
  bool isBleAvailable = isBtMainEnabled || isBtScanEnabled;
  if (!mIsBleAvailable.has_value() || (isBleAvailable != mIsBleAvailable)) {
    mConnection.sendSettingChangedNotification(
        fbs::Setting::BLE_AVAILABLE, toFbsSettingState(isBleAvailable));
    mIsBleAvailable = isBleAvailable;
  }

  return ndk::ScopedAStatus::ok();
}

ScopedAStatus ContextHub::queryNanoapps(int32_t contextHubId) {
  if (contextHubId != kDefaultHubId) {
    ALOGE("Invalid ID %" PRId32, contextHubId);
    return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
  }
  return toServiceSpecificError(mConnection.queryNanoapps());
}

::ndk::ScopedAStatus ContextHub::getPreloadedNanoappIds(
    std::vector<int64_t> *out_preloadedNanoappIds) {
  if (out_preloadedNanoappIds == nullptr) {
    return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
  }

  std::unique_lock<std::mutex> lock(mPreloadedNanoappIdsMutex);
  if (mPreloadedNanoappIds.has_value()) {
    for (auto iter = mPreloadedNanoappIds->begin();
         iter != mPreloadedNanoappIds->end(); ++iter) {
      out_preloadedNanoappIds->push_back(*iter);
    }
    return ScopedAStatus::ok();
  }

  std::vector<int64_t> preloadedNanoappIds;
  if (!getPreloadedNanoappIdsFromConfigFile(preloadedNanoappIds)) {
    return ScopedAStatus::fromExceptionCode(EX_SERVICE_SPECIFIC);
  }

  mPreloadedNanoappIds = preloadedNanoappIds;
  for (auto iter = mPreloadedNanoappIds->begin();
       iter != mPreloadedNanoappIds->end(); ++iter) {
    out_preloadedNanoappIds->push_back(*iter);
  }
  return ScopedAStatus::ok();
}

ScopedAStatus ContextHub::registerCallback(
    int32_t contextHubId, const std::shared_ptr<IContextHubCallback> &cb) {
  if (contextHubId != kDefaultHubId) {
    ALOGE("Invalid ID %" PRId32, contextHubId);
    return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
  }
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  if (mCallback != nullptr) {
    binder_status_t binder_status = AIBinder_unlinkToDeath(
        mCallback->asBinder().get(), mDeathRecipient.get(), this);
    if (binder_status != STATUS_OK) {
      ALOGE("Failed to unlink to death");
    }
  }
  mCallback = cb;
  if (cb != nullptr) {
    binder_status_t binder_status =
        AIBinder_linkToDeath(cb->asBinder().get(), mDeathRecipient.get(), this);
    if (binder_status != STATUS_OK) {
      ALOGE("Failed to link to death");
    }
  }
  return ScopedAStatus::ok();
}

ScopedAStatus ContextHub::sendMessageToHub(int32_t contextHubId,
                                           const ContextHubMessage &message) {
  if (contextHubId != kDefaultHubId) {
    ALOGE("Invalid ID %" PRId32, contextHubId);
    return ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
  }

  const bool success = mConnection.sendMessageToHub(
      message.nanoappId, message.messageType, message.hostEndPoint,
      message.messageBody.data(), message.messageBody.size());
  mEventLogger.logMessageToNanoapp(message, success);

  return toServiceSpecificError(success);
}

ScopedAStatus ContextHub::onHostEndpointConnected(
    const HostEndpointInfo &in_info) {
  std::lock_guard<std::mutex> lock(mConnectedHostEndpointsMutex);
  mConnectedHostEndpoints.insert(in_info.hostEndpointId);

  uint8_t type = (in_info.type == HostEndpointInfo::Type::FRAMEWORK)
                     ? CHRE_HOST_ENDPOINT_TYPE_FRAMEWORK
                     : CHRE_HOST_ENDPOINT_TYPE_APP;

  mConnection.onHostEndpointConnected(
      in_info.hostEndpointId, type, in_info.packageName.value_or(std::string()),
      in_info.attributionTag.value_or(std::string()));

  return ndk::ScopedAStatus::ok();
}

ScopedAStatus ContextHub::onHostEndpointDisconnected(
    char16_t in_hostEndpointId) {
  std::lock_guard<std::mutex> lock(mConnectedHostEndpointsMutex);
  if (mConnectedHostEndpoints.count(in_hostEndpointId) > 0) {
    mConnectedHostEndpoints.erase(in_hostEndpointId);

    mConnection.onHostEndpointDisconnected(in_hostEndpointId);
  } else {
    ALOGE("Unknown host endpoint disconnected (ID: %" PRIu16 ")",
          in_hostEndpointId);
  }

  return ndk::ScopedAStatus::ok();
}

void ContextHub::onNanoappMessage(const ::chre::fbs::NanoappMessageT &message) {
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  if (mCallback != nullptr) {
    mEventLogger.logMessageFromNanoapp(message);
    ContextHubMessage outMessage;
    outMessage.nanoappId = message.app_id;
    outMessage.hostEndPoint = message.host_endpoint;
    outMessage.messageType = message.message_type;
    outMessage.messageBody = message.message;
    outMessage.permissions = chreToAndroidPermissions(message.permissions);

    std::vector<std::string> messageContentPerms =
        chreToAndroidPermissions(message.message_permissions);
    mCallback->handleContextHubMessage(outMessage, messageContentPerms);
  }
}

void ContextHub::onNanoappListResponse(
    const ::chre::fbs::NanoappListResponseT &response) {
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  if (mCallback == nullptr) {
    return;
  }
  std::vector<NanoappInfo> appInfoList;

  for (const std::unique_ptr<::chre::fbs::NanoappListEntryT> &nanoapp :
       response.nanoapps) {
    // TODO(b/245202050): determine if this is really required, and if so, have
    // HostProtocolHost strip out null entries as part of decode
    if (nanoapp == nullptr) {
      continue;
    }

    ALOGV("App 0x%016" PRIx64 " ver 0x%" PRIx32 " permissions 0x%" PRIx32
          " enabled %d system %d",
          nanoapp->app_id, nanoapp->version, nanoapp->permissions,
          nanoapp->enabled, nanoapp->is_system);
    if (!nanoapp->is_system) {
      NanoappInfo appInfo;

      appInfo.nanoappId = nanoapp->app_id;
      appInfo.nanoappVersion = nanoapp->version;
      appInfo.enabled = nanoapp->enabled;
      appInfo.permissions = chreToAndroidPermissions(nanoapp->permissions);

      std::vector<NanoappRpcService> rpcServices;
      for (const auto &service : nanoapp->rpc_services) {
        NanoappRpcService aidlService;
        aidlService.id = service->id;
        aidlService.version = service->version;
        rpcServices.emplace_back(aidlService);
      }
      appInfo.rpcServices = rpcServices;

      appInfoList.push_back(appInfo);
    }
  }

  mCallback->handleNanoappInfo(appInfoList);
}

void ContextHub::onTransactionResult(uint32_t transactionId, bool success) {
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  if (mCallback != nullptr) {
    mCallback->handleTransactionResult(transactionId, success);
  }
}

void ContextHub::onContextHubRestarted() {
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  mIsWifiAvailable.reset();
  {
    std::lock_guard<std::mutex> endpointLock(mConnectedHostEndpointsMutex);
    mConnectedHostEndpoints.clear();
    mEventLogger.logContextHubRestart();
  }
  if (mCallback != nullptr) {
    mCallback->handleContextHubAsyncEvent(AsyncEventType::RESTARTED);
  }
}

void ContextHub::onDebugDumpData(const ::chre::fbs::DebugDumpDataT &data) {
  auto str = std::string(reinterpret_cast<const char *>(data.debug_str.data()),
                         data.debug_str.size());
  debugDumpAppend(str);
}

void ContextHub::onDebugDumpComplete(
    const ::chre::fbs::DebugDumpResponseT & /* response */) {
  debugDumpComplete();
}

void ContextHub::handleServiceDeath() {
  ALOGI("Context Hub Service died ...");
  {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mCallback.reset();
  }
  {
    std::lock_guard<std::mutex> lock(mConnectedHostEndpointsMutex);
    mConnectedHostEndpoints.clear();
  }
}

void ContextHub::onServiceDied(void *cookie) {
  auto *contexthub = static_cast<ContextHub *>(cookie);
  contexthub->handleServiceDeath();
}

binder_status_t ContextHub::dump(int fd, const char ** /* args */,
                                 uint32_t /* numArgs */) {
  debugDumpStart(fd);
  debugDumpFinish();
  return STATUS_OK;
}

void ContextHub::debugDumpFinish() {
  if (checkDebugFd()) {
    const std::string &dump = mEventLogger.dump();
    writeToDebugFile(dump.c_str());
    writeToDebugFile("\n-- End of CHRE/ASH debug info --\n");
    invalidateDebugFd();
  }
}

void ContextHub::writeToDebugFile(const char *str) {
  if (!::android::base::WriteStringToFd(std::string(str), getDebugFd())) {
    ALOGW("Failed to write %zu bytes to debug dump fd", strlen(str));
  }
}

bool ContextHub::getPreloadedNanoappIdsFromConfigFile(
    std::vector<int64_t> &preloadedNanoappIds) const {
  std::string directory;
  std::vector<std::string> nanoapps;
  std::string errorString;
  bool success = getPreloadedNanoappsFromConfigFile(
      kPreloadedNanoappsConfigPath, directory, nanoapps, errorString);
  if (!success) {
    ALOGE("%s", errorString.c_str());
    ALOGE("Failed to parse preloaded nanoapps config file");
  }

  for (const std::string &nanoapp : nanoapps) {
    std::string headerFile = directory + "/" + nanoapp + ".napp_header";
    std::vector<uint8_t> headerBuffer;
    if (!readFileContents(headerFile.c_str(), &headerBuffer)) {
      ALOGE("Cannot read header file: %s", headerFile.c_str());
      continue;
    }

    if (headerBuffer.size() != sizeof(NanoAppBinaryHeader)) {
      ALOGE("Header size mismatch");
      continue;
    }

    const auto *appHeader =
        reinterpret_cast<const NanoAppBinaryHeader *>(headerBuffer.data());
    preloadedNanoappIds.push_back(appHeader->appId);
  }
  return true;
}

}  // namespace aidl::android::hardware::contexthub
