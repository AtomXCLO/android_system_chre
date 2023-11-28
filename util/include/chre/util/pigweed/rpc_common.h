/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef CHRE_UTIL_PIGWEED_RPC_COMMON_H_
#define CHRE_UTIL_PIGWEED_RPC_COMMON_H_

#include <cstdint>

/**
 * @file
 * Common definitions across nanoapp and host sides.
 */

namespace chre {

// Random value chosen that matches Java client util, but is random enough
// to not conflict with other CHRE messages the nanoapp and client may send.
static constexpr uint32_t PW_RPC_CHRE_HOST_MESSAGE_TYPE = INT32_MAX - 10;

// Random values chosen to be towards the end of the nanoapp event type region
// so it doesn't conflict with existing nanoapp messages that can be sent.
static constexpr uint16_t PW_RPC_CHRE_NAPP_REQUEST_EVENT_TYPE = UINT16_MAX - 10;
static constexpr uint16_t PW_RPC_CHRE_NAPP_RESPONSE_EVENT_TYPE = UINT16_MAX - 9;

/** The upper 16b of a channel ID are set to 1 for host clients. */
constexpr uint32_t kChannelIdHostClient = 1 << 16;

/** Mask to extract the host ID / nanoapp ID from a channel ID. */
constexpr uint32_t kRpcClientIdMask = 0xffff;

/** Maximum ID for a nanoapp as the value is encoded on 16b. */
constexpr uint32_t kRpcNanoappMaxId = 0xffff;

}  // namespace chre

#endif  // CHRE_UTIL_PIGWEED_RPC_COMMON_H_