#include "cave_talk.h"

#include <cstddef>
#include <functional>

#include "air_quality.pb.h"
#include "arm.pb.h"
#include "camera_movement.pb.h"
#include "cave_talk_link.h"
#include "cave_talk_types.h"
#include "config_encoder.pb.h"
#include "config_log.pb.h"
#include "config_motor.pb.h"
#include "config_pid.pb.h"
#include "config_servo.pb.h"
#include "ids.pb.h"
#include "lights.pb.h"
#include "log.pb.h"
#include "movement.pb.h"
#include "odometry.pb.h"
#include "ooga_booga.pb.h"

namespace cave_talk
{

ListenerCallbacks::~ListenerCallbacks() = default;

Listener::Listener(CaveTalk_Error_t (*receive)(void *const data, const size_t size, size_t *const bytes_received),
                   std::shared_ptr<ListenerCallbacks> listener_callbacks) : listener_callbacks_(listener_callbacks)
{
    link_handle_         = kCaveTalk_LinkHandleNull;
    link_handle_.send    = nullptr;
    link_handle_.receive = receive;
}

CaveTalk_Error_t Listener::Listen(void)
{
    CaveTalk_Id_t     id     = 0U;
    CaveTalk_Length_t length = 0U;
    CaveTalk_Error_t  error  = CaveTalk_Listen(&link_handle_, &id, buffer_.data(), buffer_.size(), &length);

    if (CAVE_TALK_ERROR_NONE == error)
    {
        switch (static_cast<Id>(id))
        {
        case ID_NONE:
            if (0U != length)
            {
                error = CAVE_TALK_ERROR_ID;
            }
            break;
        case ID_OOGA:
            error = HandleOogaBooga(length);
            break;
        case ID_MOVEMENT:
            error = HandleMovement(length);
            break;
        case ID_CAMERA_MOVEMENT:
            error = HandleCameraMovement(length);
            break;
        case ID_LIGHTS:
            error = HandleLights(length);
            break;
        case ID_ARM:
            error = HandleArm(length);
            break;
        case ID_ODOMETRY:
            error = HandleOdometry(length);
            break;
        case ID_LOG:
            error = HandleLog(length);
            break;
        case ID_CONFIG_SERVO_WHEELS:
            error = HandleConfigServoWheels(length);
            break;
        case ID_CONFIG_SERVO_CAMS:
            error = HandleConfigServoCams(length);
            break;
        case ID_CONFIG_MOTOR:
            error = HandleConfigMotor(length);
            break;
        case ID_CONFIG_ENCODER:
            error = HandleConfigEncoder(length);
            break;
        case ID_CONFIG_LOG:
            error = HandleConfigLog(length);
            break;
        case ID_CONFIG_WHEEL_SPEED_CONTROL:
            error = HandleConfigWheelSpeedControl(length);
            break;
        case ID_CONFIG_STEERING_CONTROL:
            error = HandleConfigSteeringControl(length);
            break;
        case ID_AIR_QUALITY:
            error = HandleAirQuality(length);
            break;
        default:
            error = CAVE_TALK_ERROR_ID;
            break;
        }
    }

    return error;
}

CaveTalk_Error_t Listener::HandleOogaBooga(CaveTalk_Length_t length) const
{
    OogaBooga ooga_booga_message;

    if (!ooga_booga_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const Say ooga_booga = ooga_booga_message.ooga_booga();

    listener_callbacks_->HearOogaBooga(ooga_booga);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleMovement(CaveTalk_Length_t length) const
{
    Movement movement_message;

    if (!movement_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const CaveTalk_MetersPerSecond_t  speed     = movement_message.speed_meters_per_second();
    const CaveTalk_RadiansPerSecond_t turn_rate = movement_message.turn_rate_radians_per_second();

    listener_callbacks_->HearMovement(speed, turn_rate);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleCameraMovement(CaveTalk_Length_t length) const
{
    CameraMovement camera_movement_message;

    if (!camera_movement_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const CaveTalk_Radian_t pan  = camera_movement_message.pan_angle_radians();
    const CaveTalk_Radian_t tilt = camera_movement_message.tilt_angle_radians();

    listener_callbacks_->HearCameraMovement(pan, tilt);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleLights(CaveTalk_Length_t length) const
{
    Lights lights_message;

    if (!lights_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const bool headlights = lights_message.headlights();

    listener_callbacks_->HearLights(headlights);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleArm(CaveTalk_Length_t length) const
{
    Arm arm_message;

    if (!arm_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const bool arm = arm_message.arm();

    listener_callbacks_->HearArm(arm);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleLog(CaveTalk_Length_t length) const
{
    Log log_message;

    if (!log_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const char *const log = (log_message.log_string()).c_str();

    listener_callbacks_->HearLog(log);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleOdometry(CaveTalk_Length_t length) const
{

    Odometry odometry_message;

    if (!odometry_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const Imu     IMU             = odometry_message.imu();
    const Encoder encoder_wheel_0 = odometry_message.encoder_wheel_0();
    const Encoder encoder_wheel_1 = odometry_message.encoder_wheel_1();
    const Encoder encoder_wheel_2 = odometry_message.encoder_wheel_2();
    const Encoder encoder_wheel_3 = odometry_message.encoder_wheel_3();

    listener_callbacks_->HearOdometry(IMU, encoder_wheel_0, encoder_wheel_1, encoder_wheel_2, encoder_wheel_3);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleConfigServoWheels(CaveTalk_Length_t length) const
{
    ConfigServoWheels config_servo_wheels_message;

    if (!config_servo_wheels_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const Servo servo_wheel_0 = config_servo_wheels_message.servo_wheel_0();
    const Servo servo_wheel_1 = config_servo_wheels_message.servo_wheel_1();
    const Servo servo_wheel_2 = config_servo_wheels_message.servo_wheel_2();
    const Servo servo_wheel_3 = config_servo_wheels_message.servo_wheel_3();

    listener_callbacks_->HearConfigServoWheels(servo_wheel_0, servo_wheel_1, servo_wheel_2, servo_wheel_3);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleConfigServoCams(CaveTalk_Length_t length) const
{
    ConfigServoCams config_servo_cams_message;

    if (!config_servo_cams_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const Servo servo_cam_pan  = config_servo_cams_message.servo_cam_pan();
    const Servo servo_cam_tilt = config_servo_cams_message.servo_cam_tilt();

    listener_callbacks_->HearConfigServoCams(servo_cam_pan, servo_cam_tilt);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleConfigMotor(CaveTalk_Length_t length) const
{
    ConfigMotor config_motor_message;

    if (!config_motor_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const Motor motor_wheel_0 = config_motor_message.motor_wheel_0();
    const Motor motor_wheel_1 = config_motor_message.motor_wheel_1();
    const Motor motor_wheel_2 = config_motor_message.motor_wheel_2();
    const Motor motor_wheel_3 = config_motor_message.motor_wheel_3();

    listener_callbacks_->HearConfigMotor(motor_wheel_0, motor_wheel_1, motor_wheel_2, motor_wheel_3);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleConfigEncoder(CaveTalk_Length_t length) const
{
    ConfigEncoders config_encoder_message;

    if (!config_encoder_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const ConfigEncoder encoder_wheel_0 = config_encoder_message.encoder_wheel_0();
    const ConfigEncoder encoder_wheel_1 = config_encoder_message.encoder_wheel_1();
    const ConfigEncoder encoder_wheel_2 = config_encoder_message.encoder_wheel_2();
    const ConfigEncoder encoder_wheel_3 = config_encoder_message.encoder_wheel_3();

    listener_callbacks_->HearConfigEncoder(encoder_wheel_0, encoder_wheel_1, encoder_wheel_2, encoder_wheel_3);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleConfigLog(CaveTalk_Length_t length) const
{
    ConfigLog config_log_message;

    if (!config_log_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const LogLevel log_level = config_log_message.log_level();

    listener_callbacks_->HearConfigLog(log_level);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleConfigWheelSpeedControl(CaveTalk_Length_t length) const
{
    ConfigWheelSpeedControl config_wsc_message;

    if (!config_wsc_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const PID  wheel_0_params = config_wsc_message.wheel_0_params();
    const PID  wheel_1_params = config_wsc_message.wheel_1_params();
    const PID  wheel_2_params = config_wsc_message.wheel_2_params();
    const PID  wheel_3_params = config_wsc_message.wheel_3_params();
    const bool enabled        = config_wsc_message.enabled();

    listener_callbacks_->HearConfigWheelSpeedControl(wheel_0_params, wheel_1_params, wheel_2_params, wheel_3_params, enabled);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleConfigSteeringControl(CaveTalk_Length_t length) const
{
    ConfigSteeringControl config_sc_message;

    if (!config_sc_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const PID  turn_rate_params = config_sc_message.turn_rate_params();
    const bool enabled          = config_sc_message.enabled();

    listener_callbacks_->HearConfigSteeringControl(turn_rate_params, enabled);

    return CAVE_TALK_ERROR_NONE;
}

CaveTalk_Error_t Listener::HandleAirQuality(CaveTalk_Length_t length) const
{
    AirQuality air_quality_message;

    if (!air_quality_message.ParseFromArray(buffer_.data(), length))
    {
        return CAVE_TALK_ERROR_PARSE;
    }

    const uint32_t dust_ug_per_m3      = air_quality_message.dust_ug_per_m3();
    const uint32_t gas_ppm             = air_quality_message.gas_ppm();
    const double   temperature_celsius = air_quality_message.temperature_celsius();

    listener_callbacks_->HearAirQuality(dust_ug_per_m3, gas_ppm, temperature_celsius);

    return CAVE_TALK_ERROR_NONE;
}

Talker::Talker(CaveTalk_Error_t (*send)(const void *const data, const size_t size))
{
    link_handle_         = kCaveTalk_LinkHandleNull;
    link_handle_.send    = send;
    link_handle_.receive = nullptr;
}

CaveTalk_Error_t Talker::SpeakOogaBooga(const Say ooga_booga)
{
    OogaBooga ooga_booga_message;
    ooga_booga_message.set_ooga_booga(ooga_booga);

    std::size_t length = ooga_booga_message.ByteSizeLong();
    ooga_booga_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_OOGA), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakMovement(const CaveTalk_MetersPerSecond_t speed, const CaveTalk_RadiansPerSecond_t turn_rate)
{
    Movement movement_message;
    movement_message.set_speed_meters_per_second(speed);
    movement_message.set_turn_rate_radians_per_second(turn_rate);

    std::size_t length = movement_message.ByteSizeLong();
    movement_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_MOVEMENT), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakCameraMovement(const CaveTalk_Radian_t pan, const CaveTalk_Radian_t tilt)
{
    CameraMovement camera_movement_message;
    camera_movement_message.set_pan_angle_radians(pan);
    camera_movement_message.set_tilt_angle_radians(tilt);

    std::size_t length = camera_movement_message.ByteSizeLong();
    camera_movement_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_CAMERA_MOVEMENT), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakLights(const bool headlights)
{
    Lights lights_message;
    lights_message.set_headlights(headlights);

    std::size_t length = lights_message.ByteSizeLong();
    lights_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_LIGHTS), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakArm(const bool arm)
{
    Arm arm_message;
    arm_message.set_arm(arm);

    std::size_t length = arm_message.ByteSizeLong();
    arm_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_ARM), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakLog(const char *const log)
{
    Log log_message;
    log_message.set_log_string(log);

    std::size_t length = log_message.ByteSizeLong();
    log_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_LOG), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakOdometry(const Imu &IMU, const Encoder &encoder_wheel_0, const Encoder &encoder_wheel_1, const Encoder &encoder_wheel_2, const Encoder &encoder_wheel_3)
{
    Odometry odometry_message;
    odometry_message.mutable_imu()->CopyFrom(IMU);
    odometry_message.mutable_encoder_wheel_0()->CopyFrom(encoder_wheel_0);
    odometry_message.mutable_encoder_wheel_1()->CopyFrom(encoder_wheel_1);
    odometry_message.mutable_encoder_wheel_2()->CopyFrom(encoder_wheel_2);
    odometry_message.mutable_encoder_wheel_3()->CopyFrom(encoder_wheel_3);

    size_t length = odometry_message.ByteSizeLong();
    odometry_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_ODOMETRY), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakConfigServoWheels(const Servo &servo_wheel_0, const Servo &servo_wheel_1, const Servo &servo_wheel_2, const Servo &servo_wheel_3)
{
    ConfigServoWheels config_servo_wheels_message;

    config_servo_wheels_message.mutable_servo_wheel_0()->CopyFrom(servo_wheel_0);
    config_servo_wheels_message.mutable_servo_wheel_1()->CopyFrom(servo_wheel_1);
    config_servo_wheels_message.mutable_servo_wheel_2()->CopyFrom(servo_wheel_2);
    config_servo_wheels_message.mutable_servo_wheel_3()->CopyFrom(servo_wheel_3);

    std::size_t length = config_servo_wheels_message.ByteSizeLong();
    config_servo_wheels_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_CONFIG_SERVO_WHEELS), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakConfigServoCams(const Servo &servo_cam_pan, const Servo &servo_cam_tilt)
{

    ConfigServoCams config_servo_cams_message;

    config_servo_cams_message.mutable_servo_cam_pan()->CopyFrom(servo_cam_pan);
    config_servo_cams_message.mutable_servo_cam_tilt()->CopyFrom(servo_cam_tilt);

    std::size_t length = config_servo_cams_message.ByteSizeLong();
    config_servo_cams_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_CONFIG_SERVO_CAMS), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakConfigMotor(const Motor &motor_wheel_0, const Motor &motor_wheel_1, const Motor &motor_wheel_2, const Motor &motor_wheel_3)
{
    ConfigMotor config_motor_message;

    config_motor_message.mutable_motor_wheel_0()->CopyFrom(motor_wheel_0);
    config_motor_message.mutable_motor_wheel_1()->CopyFrom(motor_wheel_1);
    config_motor_message.mutable_motor_wheel_2()->CopyFrom(motor_wheel_2);
    config_motor_message.mutable_motor_wheel_3()->CopyFrom(motor_wheel_3);

    std::size_t length = config_motor_message.ByteSizeLong();
    config_motor_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_CONFIG_MOTOR), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakConfigEncoder(const ConfigEncoder &encoder_wheel_0, const ConfigEncoder &encoder_wheel_1, const ConfigEncoder &encoder_wheel_2, const ConfigEncoder &encoder_wheel_3)
{
    ConfigEncoders config_encoder_message;

    config_encoder_message.mutable_encoder_wheel_0()->CopyFrom(encoder_wheel_0);
    config_encoder_message.mutable_encoder_wheel_1()->CopyFrom(encoder_wheel_1);
    config_encoder_message.mutable_encoder_wheel_2()->CopyFrom(encoder_wheel_2);
    config_encoder_message.mutable_encoder_wheel_3()->CopyFrom(encoder_wheel_3);

    std::size_t length = config_encoder_message.ByteSizeLong();
    config_encoder_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_CONFIG_ENCODER), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakConfigLog(const LogLevel log_level)
{
    ConfigLog config_log_message;
    config_log_message.set_log_level(log_level);

    std::size_t length = config_log_message.ByteSizeLong();
    config_log_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_CONFIG_LOG), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakConfigWheelSpeedControl(const PID &wheel_0_params, const PID &wheel_1_params, const PID &wheel_2_params, const PID &wheel_3_params, const bool enabled)
{
    ConfigWheelSpeedControl config_wsc_message;

    config_wsc_message.mutable_wheel_0_params()->CopyFrom(wheel_0_params);
    config_wsc_message.mutable_wheel_1_params()->CopyFrom(wheel_1_params);
    config_wsc_message.mutable_wheel_2_params()->CopyFrom(wheel_2_params);
    config_wsc_message.mutable_wheel_3_params()->CopyFrom(wheel_3_params);
    config_wsc_message.set_enabled(enabled);

    std::size_t length = config_wsc_message.ByteSizeLong();
    config_wsc_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_CONFIG_WHEEL_SPEED_CONTROL), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakConfigSteeringControl(const PID &turn_rate_params, const bool enabled)
{
    ConfigSteeringControl config_sc_message;

    config_sc_message.mutable_turn_rate_params()->CopyFrom(turn_rate_params);
    config_sc_message.set_enabled(enabled);

    std::size_t length = config_sc_message.ByteSizeLong();
    config_sc_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_CONFIG_STEERING_CONTROL), message_buffer_.data(), length);
}

CaveTalk_Error_t Talker::SpeakAirQuality(const uint32_t dust_ug_per_m3, const uint32_t gas_ppm, const double temperature_celsius)
{
    AirQuality air_quality_message;
    air_quality_message.set_dust_ug_per_m3(dust_ug_per_m3);
    air_quality_message.set_gas_ppm(gas_ppm);
    air_quality_message.set_temperature_celsius(temperature_celsius);

    std::size_t length = air_quality_message.ByteSizeLong();
    air_quality_message.SerializeToArray(message_buffer_.data(), message_buffer_.max_size());

    return CaveTalk_Speak(&link_handle_, static_cast<CaveTalk_Id_t>(ID_AIR_QUALITY), message_buffer_.data(), length);
}

} // namespace cave_talk