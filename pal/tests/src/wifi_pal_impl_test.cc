/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "wifi_pal_impl_test.h"

#include "chre/platform/log.h"
#include "chre/platform/shared/pal_system_api.h"
#include "chre/platform/system_time.h"
#include "chre/util/lock_guard.h"
#include "chre/util/nanoapp/wifi.h"
#include "chre/util/time.h"

#include <cinttypes>

namespace {

using ::chre::Nanoseconds;
using ::chre::Seconds;
using ::chre::SystemTime;

//! A pointer to the current test running
wifi_pal_impl_test::PalWifiTest *gTest = nullptr;

void chrePalScanMonitorStatusChangeCallback(bool enabled, uint8_t errorCode) {
  if (gTest != nullptr) {
    gTest->scanMonitorStatusChangeCallback(enabled, errorCode);
  }
}

void chrePalScanResponseCallback(bool pending, uint8_t errorCode) {
  if (gTest != nullptr) {
    gTest->scanResponseCallback(pending, errorCode);
  }
}

void chrePalScanEventCallback(struct chreWifiScanEvent *event) {
  if (gTest != nullptr) {
    gTest->scanEventCallback(event);
  }
}

void chrePalRangingEventCallback(uint8_t errorCode,
                                 struct chreWifiRangingEvent *event) {
  if (gTest != nullptr) {
    gTest->rangingEventCallback(errorCode, event);
  }
}

void logChreWifiResult(const chreWifiScanResult &result) {
  const char *ssidStr = "<non-printable>";
  char ssidBuffer[chre::kMaxSsidStrLen];
  if (result.ssidLen == 0) {
    ssidStr = "<empty>";
  } else if (chre::parseSsidToStr(ssidBuffer, sizeof(ssidBuffer), result.ssid,
                                  result.ssidLen)) {
    ssidStr = ssidBuffer;
  }

  LOGI("Found network with SSID: %s", ssidStr);
  const char *bssidStr = "<non-printable>";
  char bssidBuffer[chre::kBssidStrLen];
  if (chre::parseBssidToStr(result.bssid, bssidBuffer, sizeof(bssidBuffer))) {
    bssidStr = bssidBuffer;
  }

  LOGI("  age (ms): %" PRIu32, result.ageMs);
  LOGI("  capability info: 0x%" PRIx16, result.capabilityInfo);
  LOGI("  bssid: %s", bssidStr);
  LOGI("  flags: 0x%" PRIx8, result.flags);
  LOGI("  rssi: %" PRId8 "dBm", result.rssi);
  LOGI("  band: %s (%" PRIu8 ")", chre::parseChreWifiBand(result.band),
       result.band);
  LOGI("  primary channel: %" PRIu32, result.primaryChannel);
  LOGI("  center frequency primary: %" PRIu32, result.centerFreqPrimary);
  LOGI("  center frequency secondary: %" PRIu32, result.centerFreqSecondary);
  LOGI("  channel width: %" PRIu8, result.channelWidth);
  LOGI("  security mode: 0x%" PRIx8, result.securityMode);
}

}  // anonymous namespace

namespace wifi_pal_impl_test {

void PalWifiTest::SetUp() {
  api_ = chrePalWifiGetApi(CHRE_PAL_WIFI_API_CURRENT_VERSION);
  ASSERT_NE(api_, nullptr);
  EXPECT_EQ(api_->moduleVersion, CHRE_PAL_WIFI_API_CURRENT_VERSION);

  // Open the PAL API
  static const struct chrePalWifiCallbacks kCallbacks = {
      .scanMonitorStatusChangeCallback = chrePalScanMonitorStatusChangeCallback,
      .scanResponseCallback = chrePalScanResponseCallback,
      .scanEventCallback = chrePalScanEventCallback,
      .rangingEventCallback = chrePalRangingEventCallback,
  };
  ASSERT_TRUE(api_->open(&chre::gChrePalSystemApi, &kCallbacks));
  gTest = this;

  errorCode_ = CHRE_ERROR_LAST;
  numScanResultCount_ = 0;
  lastScanEventReceived_ = false;
  scanEventList_.clear();
  scanParams_.reset();
  lastEventIndex_ = UINT8_MAX;
}

void PalWifiTest::TearDown() {
  gTest = nullptr;
  api_->close();
}

void PalWifiTest::scanMonitorStatusChangeCallback(bool enabled,
                                                  uint8_t errorCode) {
  // TODO:
}

void PalWifiTest::scanResponseCallback(bool pending, uint8_t errorCode) {
  LOGI("Received scan response with pending %d error %" PRIu8, pending,
       errorCode);
  chre::LockGuard<chre::Mutex> lock(mutex_);
  errorCode_ = errorCode;
  condVar_.notify_one();
}

void PalWifiTest::scanEventCallback(struct chreWifiScanEvent *event) {
  if (event == nullptr) {
    LOGE("Got null scan event");
  } else {
    {
      chre::LockGuard<chre::Mutex> lock(mutex_);
      scanEventList_.push_back(event);
      numScanResultCount_ += event->resultCount;
      lastScanEventReceived_ = (numScanResultCount_ == event->resultTotal);
    }

    condVar_.notify_one();
  }
}

void PalWifiTest::rangingEventCallback(uint8_t errorCode,
                                       struct chreWifiRangingEvent *event) {
  // TODO:
}

void PalWifiTest::validateWifiScanEvent(const chreWifiScanEvent &event) {
  if (scanParams_.has_value()) {
    EXPECT_EQ(event.scanType, scanParams_->scanType);
    EXPECT_GE(event.referenceTime,
              chreGetTime() - (scanParams_->maxScanAgeMs *
                               ::chre::kOneMillisecondInNanoseconds));
    EXPECT_EQ(event.radioChainPref, scanParams_->radioChainPref);
    EXPECT_EQ(event.eventIndex, static_cast<uint8_t>(lastEventIndex_ + 1));
  }
}

TEST_F(PalWifiTest, ScanAsyncTest) {
  // Request a WiFi scan
  chre::LockGuard<chre::Mutex> lock(mutex_);

  struct chreWifiScanParams params = {};
  params.scanType = CHRE_WIFI_SCAN_TYPE_ACTIVE;
  params.maxScanAgeMs = 5000;  // 5 seconds
  params.frequencyListLen = 0;
  params.ssidListLen = 0;
  params.radioChainPref = CHRE_WIFI_RADIO_CHAIN_PREF_DEFAULT;
  scanParams_ = params;
  ASSERT_TRUE(api_->requestScan(&scanParams_.value()));

  const Nanoseconds kTimeoutNs = Nanoseconds(CHRE_WIFI_SCAN_RESULT_TIMEOUT_NS);
  Nanoseconds end = SystemTime::getMonotonicTime() + kTimeoutNs;
  errorCode_ = CHRE_ERROR_LAST;
  while (errorCode_ == CHRE_ERROR_LAST &&
         SystemTime::getMonotonicTime() < end) {
    condVar_.wait_for(mutex_, kTimeoutNs);
  }
  ASSERT_LT(SystemTime::getMonotonicTime(), end);
  ASSERT_EQ(errorCode_, CHRE_ERROR_NONE);

  // The CHRE API only poses timeout requirements on the async response. Use
  // the same timeout to receive the scan results to avoid blocking forever.
  end = SystemTime::getMonotonicTime() + kTimeoutNs;
  while (!lastScanEventReceived_ && SystemTime::getMonotonicTime() < end) {
    condVar_.wait_for(mutex_, kTimeoutNs);
  }

  for (auto *event : scanEventList_) {
    for (uint8_t i = 0; i < event->resultCount; i++) {
      const chreWifiScanResult &result = event->results[i];
      logChreWifiResult(result);
    }
    validateWifiScanEvent(*event);

    lastEventIndex_ = event->eventIndex;
    api_->releaseScanEvent(event);
  }

  EXPECT_TRUE(lastScanEventReceived_);
  EXPECT_GT(numScanResultCount_, 0);
}

}  // namespace wifi_pal_impl_test
