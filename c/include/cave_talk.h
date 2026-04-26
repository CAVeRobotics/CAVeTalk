#ifndef CAVE_TALK_H
#define CAVE_TALK_H

#include <stdbool.h>
#include <stdint.h>

#include "air_quality.pb.h"
#include "ooga_booga.pb.h"
#include "odometry.pb.h"
#include "config_encoder.pb.h"
#include "config_log.pb.h"
#include "config_motor.pb.h"
#include "config_pid.pb.h"
#include "config_servo.pb.h"

#include "cave_talk_link.h"
#include "cave_talk_types.h"

typedef struct
{
    void (*hear_ooga_booga)(const cave_talk_Say ooga_booga);
    void (*hear_movement)(const CaveTalk_MetersPerSecond_t speed, const CaveTalk_RadiansPerSecond_t turn_rate);
    void (*hear_camera_movement)(const CaveTalk_Radian_t pan, const CaveTalk_Radian_t tilt);
    void (*hear_lights)(const bool headlights);
    void (*hear_arm)(const bool arm);
    void (*hear_odometry)(const cave_talk_Imu *const IMU, const cave_talk_Encoder *const encoder_wheel_0, const cave_talk_Encoder *const encoder_wheel_1, const cave_talk_Encoder *const encoder_wheel_2, const cave_talk_Encoder *const encoder_wheel_3);
    void (*hear_log)(const char *const log);
    void (*hear_config_servo_wheels)(const cave_talk_Servo *const servo_wheel_0, const cave_talk_Servo *const servo_wheel_1, const cave_talk_Servo *const servo_wheel_2, const cave_talk_Servo *const servo_wheel_3);
    void (*hear_config_servo_cams)(const cave_talk_Servo *const servo_cam_pan, const cave_talk_Servo *const servo_cam_tilt);
    void (*hear_config_motors)(const cave_talk_Motor *const motor_wheel_0, const cave_talk_Motor *const motor_wheel_1, const cave_talk_Motor *const motor_wheel_2, const cave_talk_Motor *const motor_wheel_3);
    void (*hear_config_encoders)(const cave_talk_ConfigEncoder *const encoder_wheel_0, const cave_talk_ConfigEncoder *const encoder_wheel_1, const cave_talk_ConfigEncoder *const encoder_wheel_2, const cave_talk_ConfigEncoder *const encoder_wheel_3);
    void (*hear_config_log)(const cave_talk_LogLevel log_level);
    void (*hear_config_wheel_speed_control)(const cave_talk_PID *const wheel_0_params, const cave_talk_PID *const wheel_1_params, const cave_talk_PID *const wheel_2_params, const cave_talk_PID *const wheel_3_params, const bool enabled);
    void (*hear_config_steering_control)(const cave_talk_PID *const turn_rate_params, const bool enabled);
    void (*hear_air_quality)(const uint32_t dust_ug_per_m3, const uint32_t gas_ppm, const double temperature_celsius);

} CaveTalk_ListenCallbacks_t;

typedef struct
{
    CaveTalk_LinkHandle_t link_handle;
    uint8_t *buffer;
    size_t buffer_size;
    CaveTalk_ListenCallbacks_t listen_callbacks;
} CaveTalk_Handle_t;

static const CaveTalk_ListenCallbacks_t kCaveTalk_ListenCallbacksNull = {
    .hear_ooga_booga                 = NULL,
    .hear_movement                   = NULL,
    .hear_camera_movement            = NULL,
    .hear_lights                     = NULL,
    .hear_arm                        = NULL,
    .hear_odometry                   = NULL,
    .hear_log                        = NULL,
    .hear_config_servo_wheels        = NULL,
    .hear_config_servo_cams          = NULL,
    .hear_config_motors              = NULL,
    .hear_config_encoders            = NULL,
    .hear_config_log                 = NULL,
    .hear_config_wheel_speed_control = NULL,
    .hear_config_steering_control    = NULL,
    .hear_air_quality                = NULL,
};

static const CaveTalk_Handle_t kCaveTalk_HandleNull = {
    .link_handle      = kCaveTalk_LinkHandleNull,
    .buffer           = NULL,
    .buffer_size      = 0U,
    .listen_callbacks = kCaveTalk_ListenCallbacksNull,
};

#ifdef __cplusplus
extern "C"
{
#endif

CaveTalk_Error_t CaveTalk_Hear(CaveTalk_Handle_t *const handle);
CaveTalk_Error_t CaveTalk_SpeakOogaBooga(const CaveTalk_Handle_t *const handle, const cave_talk_Say ooga_booga);
CaveTalk_Error_t CaveTalk_SpeakMovement(const CaveTalk_Handle_t *const handle, const CaveTalk_MetersPerSecond_t speed, const CaveTalk_RadiansPerSecond_t turn_rate);
CaveTalk_Error_t CaveTalk_SpeakCameraMovement(const CaveTalk_Handle_t *const handle, const CaveTalk_Radian_t pan, const CaveTalk_Radian_t tilt);
CaveTalk_Error_t CaveTalk_SpeakLights(const CaveTalk_Handle_t *const handle, const bool headlights);
CaveTalk_Error_t CaveTalk_SpeakArm(const CaveTalk_Handle_t *const handle, const bool arm);
CaveTalk_Error_t CaveTalk_SpeakLog(const CaveTalk_Handle_t *const handle, char *log);
CaveTalk_Error_t CaveTalk_SpeakOdometry(const CaveTalk_Handle_t *const handle, const cave_talk_Imu *const IMU, const cave_talk_Encoder *const encoder_wheel_0, const cave_talk_Encoder *const encoder_wheel_1, const cave_talk_Encoder *const encoder_wheel_2, const cave_talk_Encoder *const encoder_wheel_3);
CaveTalk_Error_t CaveTalk_SpeakConfigServoWheels(const CaveTalk_Handle_t *const handle, const cave_talk_Servo *const servo_wheel_0, const cave_talk_Servo *const servo_wheel_1, const cave_talk_Servo *const servo_wheel_2, const cave_talk_Servo *const servo_wheel_3);
CaveTalk_Error_t CaveTalk_SpeakConfigServoCams(const CaveTalk_Handle_t *const handle, const cave_talk_Servo *const servo_cam_pan, const cave_talk_Servo *const servo_cam_tilt);
CaveTalk_Error_t CaveTalk_SpeakConfigMotors(const CaveTalk_Handle_t *const handle, const cave_talk_Motor *const motor_wheel_0, const cave_talk_Motor *const motor_wheel_1, const cave_talk_Motor *const motor_wheel_2, const cave_talk_Motor *const motor_wheel_3);
CaveTalk_Error_t CaveTalk_SpeakConfigEncoders(const CaveTalk_Handle_t *const handle, const cave_talk_ConfigEncoder *const encoder_wheel_0, const cave_talk_ConfigEncoder *const encoder_wheel_1, const cave_talk_ConfigEncoder *const encoder_wheel_2, const cave_talk_ConfigEncoder *const encoder_wheel_3);
CaveTalk_Error_t CaveTalk_SpeakConfigLog(const CaveTalk_Handle_t *const handle, const cave_talk_LogLevel log_level);
CaveTalk_Error_t CaveTalk_SpeakConfigWheelSpeedControl(const CaveTalk_Handle_t *const handle, const cave_talk_PID *const wheel_0_params, const cave_talk_PID *const wheel_1_params, const cave_talk_PID *const wheel_2_params, const cave_talk_PID *const wheel_3_params, const bool enabled);
CaveTalk_Error_t CaveTalk_SpeakConfigSteeringControl(const CaveTalk_Handle_t *const handle, const cave_talk_PID *const turn_rate_params, const bool enabled);
CaveTalk_Error_t CaveTalk_SpeakAirQuality(const CaveTalk_Handle_t *const handle, const uint32_t dust_ug_per_m3, const uint32_t gas_ppm, const double temperature_celsius);

#ifdef __cplusplus
}
#endif

#endif /* CAVE_TALK_H */