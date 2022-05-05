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

#ifndef CHRE_PLATFORM_EMBOS_CONDITION_VARIABLE_IMPL_H_
#define CHRE_PLATFORM_EMBOS_CONDITION_VARIABLE_IMPL_H_

#include "chre/platform/condition_variable.h"
#include "condition_variable_base.h"

namespace chre {

inline ConditionVariable::ConditionVariable() {
  OS_SEMAPHORE_CREATE(&mCvSemaphore);
}

inline ConditionVariable::~ConditionVariable() {
  OS_SEMAPHORE_Delete(&mCvSemaphore);
}

inline void ConditionVariable::notify_one() {
  OS_SEMAPHORE_Give(&mCvSemaphore);
}

inline void ConditionVariable::wait(Mutex &mutex) {
  mutex.unlock();
  OS_SEMAPHORE_TakeBlocked(&mCvSemaphore);
  mutex.lock();
}

inline bool ConditionVariable::wait_for(Mutex &mutex, Nanoseconds timeout) {
  bool success = false;
  auto timeoutTicks =
      static_cast<OS_TIME>(Milliseconds(timeout).getMilliseconds());
  if (timeoutTicks > 0) {
    mutex.unlock();
    success = OS_SEMAPHORE_TakeTimed(&mCvSemaphore, timeoutTicks);
    mutex.lock();
  }
  return success;
}

}  // namespace chre

#endif  // CHRE_PLATFORM_EMBOS_CONDITION_VARIABLE_IMPL_H_
