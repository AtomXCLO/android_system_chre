/*
 * Copyright (C) 2016 The Android Open Source Project
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

// IWYU pragma: private, include "chre_api/chre.h"
// IWYU pragma: friend chre/.*\.h

#ifndef _CHRE_SENSOR_H_
#define _CHRE_SENSOR_H_

/**
 * @file
 * API dealing with sensor interaction in the Context Hub Runtime
 * Environment.
 *
 * This includes the definition of our sensor types and the ability to
 * configure them for receiving events.
 */

#include <stdbool.h>
#include <stdint.h>

#include <chre/common.h>
#include <chre/event.h>
#include <chre/sensor_types.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Base value for all of the data events for sensors.
 *
 * The value for a data event FOO is
 * CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_FOO
 *
 * This allows for easy mapping, and also explains why there are gaps
 * in our values since we don't have all possible sensor types assigned.
 */
#define CHRE_EVENT_SENSOR_DATA_EVENT_BASE  CHRE_EVENT_SENSOR_FIRST_EVENT

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x', 'y', and 'z' fields within
 * 'readings', or by the 3D array 'v' (v[0] == x; v[1] == y; v[2] == z).
 *
 * All values are in SI units (m/s^2) and measure the acceleration applied to
 * the device.
 */
#define CHRE_EVENT_SENSOR_ACCELEROMETER_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_ACCELEROMETER)

/**
 * nanoappHandleEvent argument: struct chreSensorOccurrenceData
 *
 * Since this is a one-shot sensor, after this event is delivered to the
 * nanoapp, the sensor automatically goes into DONE mode.  Sensors of this
 * type must be configured with a ONE_SHOT mode.
 */
#define CHRE_EVENT_SENSOR_INSTANT_MOTION_DETECT_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_INSTANT_MOTION_DETECT)

/**
 * nanoappHandleEvent argument: struct chreSensorOccurrenceData
 *
 * Since this is a one-shot sensor, after this event is delivered to the
 * nanoapp, the sensor automatically goes into DONE mode.  Sensors of this
 * type must be configured with a ONE_SHOT mode.
 */
#define CHRE_EVENT_SENSOR_STATIONARY_DETECT_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_STATIONARY_DETECT)

/**
 * nanoappHandleEvent argument: struct struct chreSensorOccurrenceData
 *
 * Since this is a one-shot sensor, after this event is delivered to the
 * nanoapp, the sensor automatically goes into DONE mode.  Sensors of this
 * type must be configured with a ONE_SHOT mode.
 */
#define CHRE_EVENT_SENSOR_SIGNIFICANT_MOTION_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_SIGNIFICANT_MOTION)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x', 'y', and 'z' fields within
 * 'readings', or by the 3D array 'v' (v[0] == x; v[1] == y; v[2] == z).
 *
 * All values are in radians/second and measure the rate of rotation
 * around the X, Y and Z axis.
 */
#define CHRE_EVENT_SENSOR_GYROSCOPE_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_GYROSCOPE)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x', 'y', and 'z' fields within
 * 'readings', or by the 3D array 'v' (v[0] == x; v[1] == y; v[2] == z).
 *
 * All values are in micro-Tesla (uT) and measure the geomagnetic
 * field in the X, Y and Z axis.
 */
#define CHRE_EVENT_SENSOR_GEOMAGNETIC_FIELD_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD)

/**
 * nanoappHandleEvent argument: struct chreSensorFloatData
 *
 * The data can be interpreted using the 'pressure' field within 'readings'.
 * This value is in hectopascals (hPa).
 */
#define CHRE_EVENT_SENSOR_PRESSURE_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_PRESSURE)

/**
 * nanoappHandleEvent argument: struct chreSensorFloatData
 *
 * The data can be interpreted using the 'light' field within 'readings'.
 * This value is in SI lux units.
 */
#define CHRE_EVENT_SENSOR_LIGHT_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_LIGHT)

/**
 * nanoappHandleEvent argument: struct chreSensorByteData
 *
 * The data is interpreted from the following fields in 'readings':
 * o 'isNear': If set to 1, we are nearby (on the order of centimeters);
 *       if set to 0, we are far. The meaning of near/far in this field must be
 *       consistent with the Android definition.
 * o 'invalid': If set to 1, this is not a valid reading of this data.
 *       As of CHRE API v1.2, this field is deprecated and must always be set to
 *       0.  If an invalid reading is generated by the sensor hardware, it must
 *       be dropped and not delivered to any nanoapp.
 *
 * In prior versions of the CHRE API, there can be an invalid event generated
 * upon configuring this sensor.  Thus, the 'invalid' field must be checked on
 * the first event before interpreting 'isNear'.
 */
#define CHRE_EVENT_SENSOR_PROXIMITY_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_PROXIMITY)

/**
 * nanoappHandleEvent argument: struct chreSensorOccurrenceData
 *
 * This data is generated every time a step is taken by the user.
 *
 * This is backed by the same algorithm that feeds Android's
 * SENSOR_TYPE_STEP_DETECTOR, and therefore sacrifices some accuracy to target
 * an update latency of under 2 seconds.
 *
 * @since v1.3
 */
#define CHRE_EVENT_SENSOR_STEP_DETECT_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_STEP_DETECT)

/**
 * nanoappHandleEvent argument: struct chreSensorUint64Data
 *
 * The value of the data is the cumulative number of steps taken by the user
 * since the last reboot while the sensor is active. This data is generated
 * every time a step is taken by the user.
 *
 * This is backed by the same algorithm that feeds Android's
 * SENSOR_TYPE_STEP_COUNTER, and therefore targets high accuracy with under
 * 10 seconds of update latency.
 *
 * @since v1.5
 */
#define CHRE_EVENT_SENSOR_STEP_COUNTER_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_STEP_COUNTER)

/**
 * nanoappHandleEvent argument: struct chreSensorFloatData
 *
 * The value of the data is the measured hinge angle between 0 and 360 degrees
 * inclusive.
 *
 * This is backed by the same algorithm that feeds Android's
 * SENSOR_TYPE_HINGE_ANGLE.
 *
 * @since v1.5
 */
#define CHRE_EVENT_SENSOR_HINGE_ANGLE_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_HINGE_ANGLE)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x', 'y', and 'z' fields within
 * 'readings', or by the 3D array 'v' (v[0] == x; v[1] == y; v[2] == z).
 *
 * All values are in SI units (m/s^2) and measure the acceleration applied to
 * the device.
 */
#define CHRE_EVENT_SENSOR_UNCALIBRATED_ACCELEROMETER_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x', 'y', and 'z' fields within
 * 'readings', or by the 3D array 'v' (v[0] == x; v[1] == y; v[2] == z).
 *
 * All values are in radians/second and measure the rate of rotation
 * around the X, Y and Z axis.
 */
#define CHRE_EVENT_SENSOR_UNCALIBRATED_GYROSCOPE_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x', 'y', and 'z' fields within
 * 'readings', or by the 3D array 'v' (v[0] == x; v[1] == y; v[2] == z).
 *
 * All values are in micro-Tesla (uT) and measure the geomagnetic
 * field in the X, Y and Z axis.
 */
#define CHRE_EVENT_SENSOR_UNCALIBRATED_GEOMAGNETIC_FIELD_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD)

/**
 * nanoappHandleEvent argument: struct chreSensorFloatData
 *
 * The data can be interpreted using the 'temperature' field within 'readings'.
 * This value is in degrees Celsius.
 */
#define CHRE_EVENT_SENSOR_ACCELEROMETER_TEMPERATURE_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_ACCELEROMETER_TEMPERATURE)

/**
 * nanoappHandleEvent argument: struct chreSensorFloatData
 *
 * The data can be interpreted using the 'temperature' field within 'readings'.
 * This value is in degrees Celsius.
 */
#define CHRE_EVENT_SENSOR_GYROSCOPE_TEMPERATURE_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_GYROSCOPE_TEMPERATURE)

/**
 * nanoappHandleEvent argument: struct chreSensorFloatData
 *
 * The data can be interpreted using the 'temperature' field within 'readings'.
 * This value is in degrees Celsius.
 */
#define CHRE_EVENT_SENSOR_GEOMAGNETIC_FIELD_TEMPERATURE_DATA \
    (CHRE_EVENT_SENSOR_DATA_EVENT_BASE + CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD_TEMPERATURE)

/**
 * First value for sensor events which are not data from the sensor.
 *
 * Unlike the data event values, these other event values don't have any
 * mapping to sensor types.
 */
#define CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE \
    (CHRE_EVENT_SENSOR_FIRST_EVENT + 0x0100)

/**
 * nanoappHandleEvent argument: struct chreSensorSamplingStatusEvent
 *
 * Indicates that the interval and/or the latency which this sensor is
 * sampling at has changed.
 */
#define CHRE_EVENT_SENSOR_SAMPLING_CHANGE \
    (CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE + 0)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x_bias', 'y_bias', and 'z_bias'
 * field within 'readings', or by the 3D array 'bias' (bias[0] == x_bias;
 * bias[1] == y_bias; bias[2] == z_bias). Bias is subtracted from uncalibrated
 * data to generate calibrated data.
 *
 * All values are in radians/second and measure the rate of rotation
 * around the X, Y and Z axis.
 *
 * If bias delivery is supported, this event is generated by default when
 * chreSensorConfigure is called to enable for the sensor of type
 * CHRE_SENSOR_TYPE_GYROSCOPE, or if bias delivery is explicitly enabled
 * through chreSensorConfigureBiasEvents() for the sensor.
 */
#define CHRE_EVENT_SENSOR_GYROSCOPE_BIAS_INFO \
    (CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE + 1)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x_bias', 'y_bias', and 'z_bias'
 * field within 'readings', or by the 3D array 'bias' (bias[0] == x_bias;
 * bias[1] == y_bias; bias[2] == z_bias). Bias is subtracted from uncalibrated
 * data to generate calibrated data.
 *
 * All values are in micro-Tesla (uT) and measure the geomagnetic
 * field in the X, Y and Z axis.
 *
 * If bias delivery is supported, this event is generated by default when
 * chreSensorConfigure is called to enable for the sensor of type
 * CHRE_SENSOR_TYPE_GEOMAGNETIC_FIELD, or if bias delivery is explicitly enabled
 * through chreSensorConfigureBiasEvents() for the sensor.
 */
#define CHRE_EVENT_SENSOR_GEOMAGNETIC_FIELD_BIAS_INFO \
    (CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE + 2)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data can be interpreted using the 'x_bias', 'y_bias', and 'z_bias'
 * field within 'readings', or by the 3D array 'bias' (bias[0] == x_bias;
 * bias[1] == y_bias; bias[2] == z_bias). Bias is subtracted from uncalibrated
 * data to generate calibrated data.
 *
 * All values are in SI units (m/s^2) and measure the acceleration applied to
 * the device.
 *
 * If bias delivery is supported, this event is generated by default when
 * chreSensorConfigure is called to enable for the sensor of type
 * CHRE_SENSOR_TYPE_ACCELEROMETER, or if bias delivery is explicitly enabled
 * through chreSensorConfigureBiasEvents() for the sensor.
 *
 * @since v1.3
 */
#define CHRE_EVENT_SENSOR_ACCELEROMETER_BIAS_INFO \
    (CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE + 3)

/**
 * nanoappHandleEvent argument: struct chreSensorFlushCompleteEvent
 *
 * An event indicating that a flush request made by chreSensorFlushAsync has
 * completed.
 *
 * @see chreSensorFlushAsync
 *
 * @since v1.3
 */
#define CHRE_EVENT_SENSOR_FLUSH_COMPLETE \
    (CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE + 4)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data of this event is the same as that of
 * CHRE_EVENT_SENSOR_GYROSCOPE_BIAS_INFO, except the sensorHandle field of
 * chreSensorDataHeader contains the handle of the sensor of type
 * CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE.
 *
 * This event is only generated if the bias reporting is explicitly enabled
 * for a nanoapp through chreSensorConfigureBiasEvents() for the sensor of type
 * CHRE_SENSOR_TYPE_UNCALIBRATED_GYROSCOPE.
 *
 * @see CHRE_EVENT_SENSOR_GYROSCOPE_BIAS_INFO
 *
 * @since v1.3
 */
#define CHRE_EVENT_SENSOR_UNCALIBRATED_GYROSCOPE_BIAS_INFO \
    (CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE + 5)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data of this event is the same as that of
 * CHRE_EVENT_SENSOR_GEOMAGNETIC_FIELD_BIAS_INFO, except the sensorHandle field
 * of chreSensorDataHeader contains the handle of the sensor of type
 * CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD.
 *
 * This event is only generated if the bias reporting is explicitly enabled
 * for a nanoapp through chreSensorConfigureBiasEvents() for the sensor of type
 * CHRE_SENSOR_TYPE_UNCALIBRATED_GEOMAGNETIC_FIELD.
 *
 * @see CHRE_EVENT_SENSOR_GEOMAGNETIC_FIELD_BIAS_INFO
 *
 * @since v1.3
 */
#define CHRE_EVENT_SENSOR_UNCALIBRATED_GEOMAGNETIC_FIELD_BIAS_INFO \
    (CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE + 6)

/**
 * nanoappHandleEvent argument: struct chreSensorThreeAxisData
 *
 * The data of this event is the same as that of
 * CHRE_EVENT_SENSOR_ACCELEROMETER_BIAS_INFO, except the sensorHandle field
 * of chreSensorDataHeader contains the handle of the sensor of type
 * CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER.
 *
 * This event is only generated if the bias reporting is explicitly enabled
 * for a nanoapp through chreSensorConfigureBiasEvents for the sensor of type
 * CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER.
 *
 * @see CHRE_EVENT_SENSOR_ACCELEROMETER_BIAS_INFO
 *
 * @since v1.3
 */
#define CHRE_EVENT_SENSOR_UNCALIBRATED_ACCELEROMETER_BIAS_INFO \
    (CHRE_EVENT_SENSOR_OTHER_EVENTS_BASE + 7)

#if CHRE_EVENT_SENSOR_UNCALIBRATED_ACCELEROMETER_BIAS_INFO > \
    CHRE_EVENT_SENSOR_LAST_EVENT
#error Too many sensor events.
#endif

/**
 * Value indicating we want the smallest possible latency for a sensor.
 *
 * This literally translates to 0 nanoseconds for the chreSensorConfigure()
 * argument.  While we won't get exactly 0 nanoseconds, the CHRE will
 * queue up this event As Soon As Possible.
 */
#define CHRE_SENSOR_LATENCY_ASAP  UINT64_C(0)

/**
 * Special value indicating non-importance, or non-applicability of the sampling
 * interval.
 *
 * @see chreSensorConfigure
 * @see chreSensorSamplingStatus
 */
#define CHRE_SENSOR_INTERVAL_DEFAULT  UINT64_C(-1)

/**
 * Special value indicating non-importance of the latency.
 *
 * @see chreSensorConfigure
 * @see chreSensorSamplingStatus
 */
#define CHRE_SENSOR_LATENCY_DEFAULT  UINT64_C(-1)

/**
 * A sensor index value indicating that it is the default sensor.
 *
 * @see chreSensorFind
 */
#define CHRE_SENSOR_INDEX_DEFAULT  UINT8_C(0)

/**
 * Special value indicating non-importance of the batch interval.
 *
 * @see chreSensorConfigureWithBatchInterval
 */
#define CHRE_SENSOR_BATCH_INTERVAL_DEFAULT  UINT64_C(-1)

// This is used to define elements of enum chreSensorConfigureMode.
#define CHRE_SENSOR_CONFIGURE_RAW_POWER_ON           (1 << 0)

// This is used to define elements of enum chreSensorConfigureMode.
#define CHRE_SENSOR_CONFIGURE_RAW_REPORT_CONTINUOUS  (1 << 1)

// This is used to define elements of enum chreSensorConfigureMode.
#define CHRE_SENSOR_CONFIGURE_RAW_REPORT_ONE_SHOT    (2 << 1)

/**
 * The maximum amount of time allowed to elapse between the call to
 * chreSensorFlushAsync() and when CHRE_EVENT_SENSOR_FLUSH_COMPLETE is delivered
 * to the nanoapp on a successful flush.
 */
#define CHRE_SENSOR_FLUSH_COMPLETE_TIMEOUT_NS  (5 * CHRE_NSEC_PER_SEC)

/**
 * Modes we can configure a sensor to use.
 *
 * Our mode will affect not only how/if we receive events, but
 * also whether or not the sensor will be powered on our behalf.
 *
 * @see chreSensorConfigure
 */
enum chreSensorConfigureMode {
    /**
     * Get events from the sensor.
     *
     * Power: Turn on if not already on.
     * Reporting: Continuous.  Send each new event as it comes (subject to
     *     batching and latency).
     */
    CHRE_SENSOR_CONFIGURE_MODE_CONTINUOUS =
        (CHRE_SENSOR_CONFIGURE_RAW_POWER_ON |
         CHRE_SENSOR_CONFIGURE_RAW_REPORT_CONTINUOUS),

    /**
     * Get a single event from the sensor and then become DONE.
     *
     * Once the event is sent, the sensor automatically
     * changes to CHRE_SENSOR_CONFIGURE_MODE_DONE mode.
     *
     * Power: Turn on if not already on.
     * Reporting: One shot.  Send the next event and then be DONE.
     */
    CHRE_SENSOR_CONFIGURE_MODE_ONE_SHOT =
        (CHRE_SENSOR_CONFIGURE_RAW_POWER_ON |
         CHRE_SENSOR_CONFIGURE_RAW_REPORT_ONE_SHOT),

    /**
     * Get events from a sensor that are generated for any client in the system.
     *
     * This is considered passive because the sensor will not be powered on for
     * the sake of our nanoapp.  If and only if another client in the system has
     * requested this sensor power on will we get events.
     *
     * This can be useful for something which is interested in seeing data, but
     * not interested enough to be responsible for powering on the sensor.
     *
     * Power: Do not power the sensor on our behalf.
     * Reporting: Continuous.  Send each event as it comes.
     */
    CHRE_SENSOR_CONFIGURE_MODE_PASSIVE_CONTINUOUS =
        CHRE_SENSOR_CONFIGURE_RAW_REPORT_CONTINUOUS,

    /**
     * Get a single event from a sensor that is generated for any client in the
     * system.
     *
     * See CHRE_SENSOR_CONFIGURE_MODE_PASSIVE_CONTINUOUS for more details on
     * what the "passive" means.
     *
     * Power: Do not power the sensor on our behalf.
     * Reporting: One shot.  Send only the next event and then be DONE.
     */
    CHRE_SENSOR_CONFIGURE_MODE_PASSIVE_ONE_SHOT =
        CHRE_SENSOR_CONFIGURE_RAW_REPORT_ONE_SHOT,

    /**
     * Indicate we are done using this sensor and no longer interested in it.
     *
     * See chreSensorConfigure for more details on expressing interest or
     * lack of interest in a sensor.
     *
     * Power: Do not power the sensor on our behalf.
     * Reporting: None.
     */
    CHRE_SENSOR_CONFIGURE_MODE_DONE = 0,
};

/**
 * A structure containing information about a Sensor.
 *
 * See documentation of individual fields below.
 */
struct chreSensorInfo {
    /**
     * The name of the sensor.
     *
     * A text name, useful for logging/debugging, describing the Sensor.  This
     * is not assured to be unique (i.e. there could be multiple sensors with
     * the name "Temperature").
     *
     * CHRE implementations may not set this as NULL.  An empty
     * string, while discouraged, is legal.
     */
    const char *sensorName;

    /**
     * One of the CHRE_SENSOR_TYPE_* defines above.
     */
    uint8_t sensorType;

    /**
     * Flag indicating if this sensor is on-change.
     *
     * An on-change sensor only generates events when underlying state
     * changes.  This has the same meaning as on-change does in the Android
     * Sensors HAL.  See sensors.h for much more details.
     *
     * A value of 1 indicates this is on-change.  0 indicates this is not
     * on-change.
     */
    uint8_t isOnChange          : 1;

    /**
     * Flag indicating if this sensor is one-shot.
     *
     * A one-shot sensor only triggers a single event, and then automatically
     * disables itself.
     *
     * A value of 1 indicates this is one-shot.  0 indicates this is not
     * on-change.
     */
    uint8_t isOneShot           : 1;

    /**
     * Flag indicating if this sensor supports reporting bias info events.
     *
     * This field will be set to 0 when running on CHRE API versions prior to
     * v1.3, but must be ignored (i.e. does not mean bias info event is not
     * supported).
     *
     * @see chreSensorConfigureBiasEvents
     *
     * @since v1.3
     */
    uint8_t reportsBiasEvents   : 1;

    /**
     * Flag indicating if this sensor supports passive mode requests.
     *
     * This field will be set to 0 when running on CHRE API versions prior to
     * v1.4, and must be ignored (i.e. does not mean passive mode requests are
     * not supported).
     *
     * @see chreSensorConfigure
     *
     * @since v1.4
     */
    uint8_t supportsPassiveMode : 1;

    uint8_t unusedFlags         : 4;

    /**
     * The minimum sampling interval supported by this sensor, in nanoseconds.
     *
     * Requests to chreSensorConfigure with a lower interval than this will
     * fail.  If the sampling interval is not applicable to this sensor, this
     * will be set to CHRE_SENSOR_INTERVAL_DEFAULT.
     *
     * This field will be set to 0 when running on CHRE API versions prior to
     * v1.1, indicating that the minimum interval is not known.
     *
     * @since v1.1
     */
    uint64_t minInterval;

    /**
     * Uniquely identifies the sensor for a given type. A value of 0 indicates
     * that this is the "default" sensor, which is returned by
     * chreSensorFindDefault().
     *
     * The sensor index of a given type must be stable across boots (i.e. must
     * not change), and a different sensor of the same type must have different
     * sensor index values, and the set of sensorIndex values for a given sensor
     * type must be continuguous.
     *
     * @since v1.5
     */
    uint8_t sensorIndex;
};

/**
 * The status of a sensor's sampling configuration.
 */
struct chreSensorSamplingStatus {
    /**
     * The interval, in nanoseconds, at which sensor data is being sampled at.
     * This should be used by nanoapps to determine the rate at which samples
     * will be generated and not to indicate what the sensor is truly sampling
     * at since resampling may occur to limit incoming data.
     *
     * If this is CHRE_SENSOR_INTERVAL_DEFAULT, then a sampling interval
     * isn't meaningful for this sensor.
     *
     * Note that if 'enabled' is false, this value is not meaningful.
     */
    uint64_t interval;

    /**
     * The latency, in nanoseconds, at which the sensor is now reporting.
     *
     * If this is CHRE_SENSOR_LATENCY_DEFAULT, then a latency
     * isn't meaningful for this sensor.
     *
     * The effective batch interval can be derived from this value by
     * adding the current sampling interval.
     *
     * Note that if 'enabled' is false, this value is not meaningful.
     */
    uint64_t latency;

    /**
     * True if the sensor is actively powered and sampling; false otherwise.
     */
    bool enabled;
};

/**
 * The nanoappHandleEvent argument for CHRE_EVENT_SENSOR_SAMPLING_CHANGE.
 *
 * Note that only at least one of 'interval' or 'latency' must be
 * different than it was prior to this event.  Thus, one of these
 * fields may be (but doesn't need to be) the same as before.
 */
struct chreSensorSamplingStatusEvent {
    /**
     * The handle of the sensor which has experienced a change in sampling.
     */
    uint32_t sensorHandle;

    /**
     * The new sampling status.
     *
     * At least one of the field in this struct will be different from
     * the previous sampling status event.
     */
    struct chreSensorSamplingStatus status;
};

/**
 * The nanoappHandleEvent argument for CHRE_EVENT_SENSOR_FLUSH_COMPLETE.
 *
 * @see chreSensorFlushAsync
 *
 * @since v1.3
 */
struct chreSensorFlushCompleteEvent {
    /**
     * The handle of the sensor which a flush was completed.
     */
    uint32_t sensorHandle;

    /**
     * Populated with a value from enum {@link #chreError}, indicating whether
     * the flush failed, and if so, provides the cause of the failure.
     */
    uint8_t errorCode;

    /**
     * Reserved for future use. Set to 0.
     */
    uint8_t reserved[3];

    /**
     * Set to the cookie parameter given to chreSensorFlushAsync.
     */
    const void *cookie;
};

/**
 * Find the default sensor for a given sensor type.
 *
 * @param sensorType One of the CHRE_SENSOR_TYPE_* constants.
 * @param handle  If a sensor is found, then the memory will be filled with
 *     the value for the sensor's handle.  This argument must be non-NULL.
 * @return true if a sensor was found, false otherwise.
 */
bool chreSensorFindDefault(uint8_t sensorType, uint32_t *handle);

/**
 * Finds a sensor of a given index and sensor type.
 *
 * For CHRE implementations that support multiple sensors of the same sensor
 * type, this method can be used to get the non-default sensor(s). The default
 * sensor, as defined in the chreSensorFindDefault(), will be returned if
 * a sensor index of zero is specified.
 *
 * A simple example of iterating all available sensors of a given type is
 * provided here:
 *
 * uint32_t handle;
 * for (uint8_t i = 0; chreSensorFind(sensorType, i, &handle); i++) {
 *   chreLog(CHRE_LOG_INFO,
 *           "Found sensor index %" PRIu8 ", which has handle %" PRIu32,
 *           i, handle);
 * }
 *
 * If this method is invoked for CHRE versions prior to v1.5, invocations with
 * sensorIndex value of 0 will be equivalent to using chreSensorFindDefault, and
 * if sensorIndex is non-zero will return false.
 *
 * In cases where multiple sensors are supported in both the Android sensors
 * framework and CHRE, the sensorName of the chreSensorInfo struct for a given
 * sensor instance must match exactly with that of the
 * android.hardware.Sensor#getName() return value. This can be used to match a
 * sensor instance between the Android and CHRE sensors APIs.
 *
 * @param sensorType One of the CHRE_SENSOR_TYPE_* constants.
 * @param sensorIndex The index of the desired sensor.
 * @param handle  If a sensor is found, then the memory will be filled with
 *     the value for the sensor's handle.  This argument must be non-NULL.
 * @return true if a sensor was found, false otherwise.
 *
 * @since v1.5
 */
bool chreSensorFind(uint8_t sensorType, uint8_t sensorIndex, uint32_t *handle);

/**
 * Get the chreSensorInfo struct for a given sensor.
 *
 * @param sensorHandle  The sensor handle, as obtained from
 *     chreSensorFindDefault() or passed to nanoappHandleEvent().
 * @param info  If the sensor is valid, then this memory will be filled with
 *     the SensorInfo contents for this sensor.  This argument must be
 *     non-NULL.
 * @return true if the senor handle is valid and 'info' was filled in;
 *     false otherwise.
 */
bool chreGetSensorInfo(uint32_t sensorHandle, struct chreSensorInfo *info);

/**
 * Get the chreSensorSamplingStatus struct for a given sensor.
 *
 * Note that this may be different from what was requested in
 * chreSensorConfigure(), for multiple reasons.  It's possible that the sensor
 * does not exactly support the interval requested in chreSensorConfigure(), so
 * a faster one was chosen.
 *
 * It's also possible that there is another user of this sensor who has
 * requested a faster interval and/or lower latency.  This latter scenario
 * should be noted, because it means the sensor rate can change due to no
 * interaction from this nanoapp.  Note that the
 * CHRE_EVENT_SENSOR_SAMPLING_CHANGE event will trigger in this case, so it's
 * not necessary to poll for such a change.
 *
 * This function must return a valid status if the provided sensor is being
 * actively sampled by a nanoapp and a CHRE_EVENT_SENSOR_SAMPLING_CHANGE has
 * been delivered indicating their request has taken effect. It is not required
 * to return a valid status if no nanoapp is actively sampling the sensor.
 *
 * @param sensorHandle  The sensor handle, as obtained from
 *     chreSensorFindDefault() or passed to nanoappHandleEvent().
 * @param status  If the sensor is actively enabled by a nanoapp, then this
 *     memory must be filled with the sampling status contents for this sensor.
 *     This argument must be non-NULL.
 * @return true if the sensor handle is valid and 'status' was filled in;
 *     false otherwise.
 */
bool chreGetSensorSamplingStatus(uint32_t sensorHandle,
                                 struct chreSensorSamplingStatus *status);

/**
 * Configures a given sensor at a specific interval and latency and mode.
 *
 * If this sensor's chreSensorInfo has isOneShot set to 1,
 * then the mode must be one of the ONE_SHOT modes, or this method will fail.
 *
 * The CHRE wants to power as few sensors as possible, in keeping with its
 * low power design.  As such, it only turns on sensors when there are clients
 * actively interested in that sensor data, and turns off sensors as soon as
 * there are no clients interested in them.  Calling this method generally
 * indicates an interest, and using CHRE_SENSOR_CONFIGURE_MODE_DONE shows
 * when we are no longer interested.
 *
 * Thus, each initial Configure of a sensor (per nanoapp) needs to eventually
 * have a DONE call made, either directly or on its behalf.  Subsequent calls
 * to a Configure method within the same nanoapp, when there has been no DONE
 * in between, still only require a single DONE call.
 *
 * For example, the following is valid usage:
 * <code>
 *   chreSensorConfigure(myHandle, mode, interval0, latency0);
 *   [...]
 *   chreSensorConfigure(myHandle, mode, interval1, latency0);
 *   [...]
 *   chreSensorConfigure(myHandle, mode, interval1, latency1);
 *   [...]
 *   chreSensorConfigureModeOnly(myHandle, CHRE_SENSOR_CONFIGURE_MODE_DONE);
 * </code>
 *
 * The first call to Configure is the one which creates the requirement
 * to eventually call with DONE.  The subsequent calls are just changing the
 * interval/latency.  They have not changed the fact that this nanoapp is
 * still interested in output from the sensor 'myHandle'.  Thus, only one
 * single call for DONE is needed.
 *
 * There is a special case.  One-shot sensors, sensors which
 * just trigger a single event and never trigger again, implicitly go into
 * DONE mode after that single event triggers.  Thus, the
 * following are legitimate usages:
 * <code>
 *   chreSensorConfigure(myHandle, MODE_ONE_SHOT, interval, latency);
 *   [...]
 *   [myHandle triggers an event]
 *   [no need to configure to DONE].
 * </code>
 *
 * And:
 * <code>
 *   chreSensorConfigure(myHandle, MODE_ONE_SHOT, interval, latency);
 *   [...]
 *   chreSensorConfigureModeOnly(myHandle, MODE_DONE);
 *   [we cancelled myHandle before it ever triggered an event]
 * </code>
 *
 * Note that while PASSIVE modes, by definition, don't express an interest in
 * powering the sensor, DONE is still necessary to silence the event reporting.
 * Starting with CHRE API v1.4, for sensors that do not support passive mode, a
 * request with mode set to CHRE_SENSOR_CONFIGURE_MODE_PASSIVE_CONTINUOUS or
 * CHRE_SENSOR_CONFIGURE_MODE_PASSIVE_ONE_SHOT will be rejected. CHRE API
 * versions 1.3 and older implicitly assume that passive mode is supported
 * across all sensors, however this is not necessarily the case. Clients can
 * call chreSensorInfo to identify whether a sensor supports passive mode.
 *
 * When a calibrated sensor (e.g. CHRE_SENSOR_TYPE_ACCELEROMETER) is
 * successfully enabled through this method and if bias delivery is supported,
 * by default CHRE will start delivering bias events for the sensor
 * (e.g. CHRE_EVENT_SENSOR_ACCELEROMETER_BIAS_INFO) to the nanoapp. If the
 * nanoapp does not wish to receive these events, they can be disabled through
 * chreSensorConfigureBiasEvents after enabling the sensor.
 *
 * @param sensorHandle  The handle to the sensor, as obtained from
 *     chreSensorFindDefault().
 * @param mode  The mode to use.  See descriptions within the
 *     chreSensorConfigureMode enum.
 * @param interval  The interval, in nanoseconds, at which we want events from
 *     the sensor.  On success, the sensor will be set to 'interval', or a value
 *     less than 'interval'.  There is a special value
 *     CHRE_SENSOR_INTERVAL_DEFAULT, in which we don't express a preference for
 *     the interval, and allow the sensor to choose what it wants.  Note that
 *     due to batching, we may receive events less frequently than
 *     'interval'.
 * @param latency  The maximum latency, in nanoseconds, allowed before the
 *     CHRE begins delivery of an event.  This will control how many events
 *     can be queued by the sensor before requiring a delivery event.
 *     Latency is defined as the "timestamp when event is queued by the CHRE"
 *     minus "timestamp of oldest unsent data reading".
 *     There is a special value CHRE_SENSOR_LATENCY_DEFAULT, in which we don't
 *     express a preference for the latency, and allow the sensor to choose what
 *     it wants.
 *     Note that there is no assurance of how long it will take an event to
 *     get through a CHRE's queueing system, and thus there is no ability to
 *     request a minimum time from the occurrence of a phenomenon to when the
 *     nanoapp receives the information.  The current CHRE API has no
 *     real-time elements, although future versions may introduce some to
 *     help with this issue.
 * @return true if the configuration succeeded, false otherwise.
 *
 * @see chreSensorConfigureMode
 * @see chreSensorFindDefault
 * @see chreSensorInfo
 * @see chreSensorConfigureBiasEvents
 */
bool chreSensorConfigure(uint32_t sensorHandle,
                         enum chreSensorConfigureMode mode,
                         uint64_t interval, uint64_t latency);

/**
 * Short cut for chreSensorConfigure where we only want to configure the mode
 * and do not care about interval/latency.
 *
 * @see chreSensorConfigure
 */
static inline bool chreSensorConfigureModeOnly(
        uint32_t sensorHandle, enum chreSensorConfigureMode mode) {
    return chreSensorConfigure(sensorHandle,
                               mode,
                               CHRE_SENSOR_INTERVAL_DEFAULT,
                               CHRE_SENSOR_LATENCY_DEFAULT);
}

/**
 * Convenience function that wraps chreSensorConfigure but enables batching to
 * be controlled by specifying the desired maximum batch interval rather
 * than maximum sample latency.  Users may find the batch interval to be a more
 * intuitive method of expressing the desired batching behavior.
 *
 * Batch interval is different from latency as the batch interval time is
 * counted starting when the prior event containing a batch of sensor samples is
 * delivered, while latency starts counting when the first sample is deferred to
 * start collecting a batch.  In other words, latency ignores the time between
 * the last sample in a batch to the first sample of the next batch, while it's
 * included in the batch interval, as illustrated below.
 *
 *  Time      0   1   2   3   4   5   6   7   8
 *  Batch             A           B           C
 *  Sample   a1  a2  a3  b1  b2  b3  c1  c2  c3
 *  Latency  [        ]  [        ]  [        ]
 *  BatchInt          |           |           |
 *
 * In the diagram, the effective sample interval is 1 time unit, latency is 2
 * time units, and batch interval is 3 time units.
 *
 * @param sensorHandle See chreSensorConfigure#sensorHandle
 * @param mode See chreSensorConfigure#mode
 * @param sampleInterval See chreSensorConfigure#interval, but note that
 *     CHRE_SENSOR_INTERVAL_DEFAULT is not a supported input to this method.
 * @param batchInterval The desired maximum interval, in nanoseconds, between
 *     CHRE enqueuing each batch of sensor samples.
 * @return Same as chreSensorConfigure
 *
 * @see chreSensorConfigure
 *
 * @since v1.1
 */
static inline bool chreSensorConfigureWithBatchInterval(
        uint32_t sensorHandle, enum chreSensorConfigureMode mode,
        uint64_t sampleInterval, uint64_t batchInterval) {
    bool result = false;

    if (sampleInterval != CHRE_SENSOR_INTERVAL_DEFAULT) {
        uint64_t latency;
        if (batchInterval == CHRE_SENSOR_BATCH_INTERVAL_DEFAULT) {
            latency = CHRE_SENSOR_LATENCY_DEFAULT;
        } else if (batchInterval > sampleInterval) {
            latency = batchInterval - sampleInterval;
        } else {
            latency = CHRE_SENSOR_LATENCY_ASAP;
        }
        result = chreSensorConfigure(sensorHandle, mode, sampleInterval,
                                     latency);
    }

    return result;
}

/**
 * Configures the reception of bias events for a specific sensor.
 *
 * If bias event delivery is supported for a sensor, the sensor's chreSensorInfo
 * has reportsBiasEvents set to 1. If supported, it must be supported for both
 * calibrated and uncalibrated versions of the sensor. If supported, CHRE must
 * provide bias events to the nanoapp by default when chreSensorConfigure is
 * called to enable the calibrated version of the sensor (for backwards
 * compatibility reasons, as this is the defined behavior for CHRE API v1.0).
 * When configuring uncalibrated sensors, nanoapps must explicitly configure an
 * enable request through this method to receive bias events. If bias event
 * delivery is not supported for the sensor, this method will return false and
 * no bias events will be generated.
 *
 * To enable bias event delivery (enable=true), the nanoapp must be registered
 * to the sensor through chreSensorConfigure, and bias events will only be
 * generated when the sensor is powered on. To disable the bias event delivery,
 * this method can be invoked with enable=false.
 *
 * If an enable configuration is successful, the calling nanoapp will receive
 * bias info events, e.g. CHRE_EVENT_SENSOR_ACCELEROMETER_BIAS_INFO, when the
 * bias status changes (or first becomes available). Calibrated data
 * (e.g. CHRE_SENSOR_TYPE_ACCELEROMETER) is generated by subracting bias from
 * uncalibrated data (e.g. CHRE_SENSOR_TYPE_UNCALIBRATED_ACCELEROMETER).
 * Calibrated sensor events are generated by applying the most recent bias
 * available (i.e. timestamp of calibrated data are greater than or equal to the
 * timestamp of the bias data that has been applied to it). The configuration of
 * bias event delivery persists until the sensor is unregistered by the nanoapp
 * through chreSensorConfigure or modified through this method.
 *
 * To get an initial bias before new bias events, the nanoapp should get the
 * bias synchronously after this method is invoked, e.g.:
 *
 * if (chreSensorConfigure(handle, ...)) {
 *   chreSensorConfigureBiasEvents(handle, true);
 *   chreSensorGetThreeAxisBias(handle, &bias);
 * }
 *
 * Note that chreSensorGetThreeAxisBias() should be called after
 * chreSensorConfigureBiasEvents() to ensure that no bias events are lost.
 *
 * If called while running on a CHRE API version below v1.3, this function
 * returns false and has no effect. The default behavior regarding bias events
 * is unchanged, meaning that the implementation may still send bias events
 * when a calibrated sensor is registered (if supported), and will not send bias
 * events when an uncalibrated sensor is registered.
 *
 * @param sensorHandle The handle to the sensor, as obtained from
 *     chreSensorFindDefault().
 * @param enable true to receive bias events, false otherwise
 *
 * @return true if the configuration succeeded, false otherwise
 *
 * @since v1.3
 */
bool chreSensorConfigureBiasEvents(uint32_t sensorHandle, bool enable);

/**
 * Synchronously provides the most recent bias info available for a sensor. The
 * bias will only be provided for a sensor that supports bias event delivery
 * using the chreSensorThreeAxisData type. If the bias is not yet available
 * (but is supported), this method will store data with a bias of 0 and the
 * accuracy field in chreSensorDataHeader set to CHRE_SENSOR_ACCURACY_UNKNOWN.
 *
 * If called while running on a CHRE API version below v1.3, this function
 * returns false.
 *
 * @param sensorHandle The handle to the sensor, as obtained from
 *     chreSensorFindDefault().
 * @param bias A pointer to where the bias will be stored.
 *
 * @return true if the bias was successfully stored, false if sensorHandle was
 *     invalid or the sensor does not support three axis bias delivery
 *
 * @since v1.3
 *
 * @see chreSensorConfigureBiasEvents
 */
bool chreSensorGetThreeAxisBias(uint32_t sensorHandle,
                                struct chreSensorThreeAxisData *bias);

/**
 * Makes a request to flush all samples stored for batching. The nanoapp must be
 * registered to the sensor through chreSensorConfigure, and the sensor must be
 * powered on. If the request is accepted, all batched samples of the sensor
 * are sent to nanoapps registered to the sensor. During a flush, it is treated
 * as though the latency as given in chreSensorConfigure has expired. When all
 * batched samples have been flushed (or the flush fails), the nanoapp will
 * receive a unicast CHRE_EVENT_SENSOR_FLUSH_COMPLETE event. The time to deliver
 * this event must not exceed CHRE_SENSOR_FLUSH_COMPLETE_TIMEOUT_NS after this
 * method is invoked. If there are no samples in the batch buffer (either in
 * hardware FIFO or software), then this method will return true and a
 * CHRE_EVENT_SENSOR_FLUSH_COMPLETE event is delivered immediately.
 *
 * If a flush request is invalid (e.g. the sensor refers to a one-shot sensor,
 * or the sensor was not enabled), and this API will return false and no
 * CHRE_EVENT_SENSOR_FLUSH_COMPLETE event will be delivered.
 *
 * If multiple flush requests are made for a sensor prior to flush completion,
 * then the requesting nanoapp will receive all batched samples existing at the
 * time of the latest flush request. In this case, the number of
 * CHRE_EVENT_SENSOR_FLUSH_COMPLETE events received must equal the number of
 * flush requests made.
 *
 * If a sensor request is disabled after a flush request is made through this
 * method but before the flush operation is completed, the nanoapp will receive
 * a CHRE_EVENT_SENSOR_FLUSH_COMPLETE with the error code
 * CHRE_ERROR_FUNCTION_DISABLED for any pending flush requests.
 *
 * Starting with CHRE API v1.3, implementations must support this capability
 * across all exposed sensor types.
 *
 * @param sensorHandle  The handle to the sensor, as obtained from
 *     chreSensorFindDefault().
 * @param cookie  An opaque value that will be included in the
 *     chreSensorFlushCompleteEvent sent in relation to this request.
 *
 * @return true if the request was accepted for processing, false otherwise
 *
 * @since v1.3
 */
bool chreSensorFlushAsync(uint32_t sensorHandle, const void *cookie);

#ifdef __cplusplus
}
#endif

#endif  /* _CHRE_SENSOR_H_ */
