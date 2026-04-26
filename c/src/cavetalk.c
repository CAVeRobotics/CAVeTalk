#include "cave_talk.h"

#include <stdbool.h>

#include "air_quality.pb.h"
#include "arm.pb.h"
#include "camera_movement.pb.h"
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
#include "pb_decode.h"
#include "pb_encode.h"

#include "cave_talk_link.h"
#include "cave_talk_types.h"

static CaveTalk_Error_t CaveTalk_HandleOogaBooga(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleMovement(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleCameraMovement(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleLights(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleArm(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleOdometry(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleLog(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleConfigServoWheels(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleConfigServoCams(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleConfigMotor(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleConfigEncoders(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleConfigLog(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleConfigWheelSpeedControl(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleConfigSteeringControl(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static CaveTalk_Error_t CaveTalk_HandleAirQuality(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length);
static bool CaveTalk_EncodeString(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
static bool CaveTalk_DecodeString(pb_istream_t *stream, const pb_field_t *field, void **arg);

CaveTalk_Error_t CaveTalk_Hear(CaveTalk_Handle_t *const handle)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) ||
        (NULL == handle->buffer) ||
        (NULL == handle->link_handle.receive))
    {
    }
    else
    {
        CaveTalk_Id_t     id     = 0U;
        CaveTalk_Length_t length = 0U;

        error = CaveTalk_Listen(&handle->link_handle, &id, handle->buffer, handle->buffer_size, &length);

        if (CAVE_TALK_ERROR_NONE == error)
        {
            switch ((cave_talk_Id)id)
            {
            case cave_talk_Id_ID_NONE:
                if (0U != length)
                {
                    error = CAVE_TALK_ERROR_ID;
                }
                break;
            case cave_talk_Id_ID_OOGA:
                error = CaveTalk_HandleOogaBooga(handle, length);
                break;
            case cave_talk_Id_ID_MOVEMENT:
                error = CaveTalk_HandleMovement(handle, length);
                break;
            case cave_talk_Id_ID_CAMERA_MOVEMENT:
                error = CaveTalk_HandleCameraMovement(handle, length);
                break;
            case cave_talk_Id_ID_LIGHTS:
                error = CaveTalk_HandleLights(handle, length);
                break;
            case cave_talk_Id_ID_ARM:
                error = CaveTalk_HandleArm(handle, length);
                break;
            case cave_talk_Id_ID_ODOMETRY:
                error = CaveTalk_HandleOdometry(handle, length);
                break;
            case cave_talk_Id_ID_LOG:
                error = CaveTalk_HandleLog(handle, length);
                break;
            case cave_talk_Id_ID_CONFIG_SERVO_WHEELS:
                error = CaveTalk_HandleConfigServoWheels(handle, length);
                break;
            case cave_talk_Id_ID_CONFIG_SERVO_CAMS:
                error = CaveTalk_HandleConfigServoCams(handle, length);
                break;
            case cave_talk_Id_ID_CONFIG_MOTOR:
                error = CaveTalk_HandleConfigMotor(handle, length);
                break;
            case cave_talk_Id_ID_CONFIG_ENCODER:
                error = CaveTalk_HandleConfigEncoders(handle, length);
                break;
            case cave_talk_Id_ID_CONFIG_LOG:
                error = CaveTalk_HandleConfigLog(handle, length);
                break;
            case cave_talk_Id_ID_CONFIG_WHEEL_SPEED_CONTROL:
                error = CaveTalk_HandleConfigWheelSpeedControl(handle, length);
                break;
            case cave_talk_Id_ID_CONFIG_STEERING_CONTROL:
                error = CaveTalk_HandleConfigSteeringControl(handle, length);
                break;
            case cave_talk_Id_ID_AIR_QUALITY:
                error = CaveTalk_HandleAirQuality(handle, length);
                break;
            default:
                error = CAVE_TALK_ERROR_ID;
                break;
            }
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakOogaBooga(const CaveTalk_Handle_t *const handle, const cave_talk_Say ooga_booga)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t        ostream            = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_OogaBooga ooga_booga_message = cave_talk_OogaBooga_init_zero;

        ooga_booga_message.ooga_booga = ooga_booga;

        if (!pb_encode(&ostream, cave_talk_OogaBooga_fields, &ooga_booga_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_OOGA, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakMovement(const CaveTalk_Handle_t *const handle, const CaveTalk_MetersPerSecond_t speed, const CaveTalk_RadiansPerSecond_t turn_rate)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t       ostream          = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_Movement movement_message = cave_talk_Movement_init_zero;

        movement_message.speed_meters_per_second      = speed;
        movement_message.turn_rate_radians_per_second = turn_rate;

        if (!pb_encode(&ostream, cave_talk_Movement_fields, &movement_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_MOVEMENT, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakCameraMovement(const CaveTalk_Handle_t *const handle, const CaveTalk_Radian_t pan, const CaveTalk_Radian_t tilt)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t             ostream                 = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_CameraMovement camera_movement_message = cave_talk_CameraMovement_init_zero;

        camera_movement_message.pan_angle_radians  = pan;
        camera_movement_message.tilt_angle_radians = tilt;

        if (!pb_encode(&ostream, cave_talk_CameraMovement_fields, &camera_movement_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_CAMERA_MOVEMENT, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakLights(const CaveTalk_Handle_t *const handle, const bool headlights)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t     ostream        = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_Lights lights_message = cave_talk_Lights_init_zero;

        lights_message.headlights = headlights;

        if (!pb_encode(&ostream, cave_talk_Lights_fields, &lights_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_LIGHTS, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakArm(const CaveTalk_Handle_t *const handle, const bool arm)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t  ostream  = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_Arm mode_arm = cave_talk_Arm_init_zero;

        mode_arm.arm = arm;

        if (!pb_encode(&ostream, cave_talk_Arm_fields, &mode_arm))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_ARM, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakOdometry(const CaveTalk_Handle_t *const handle, const cave_talk_Imu *const IMU, const cave_talk_Encoder *const encoder_wheel_0, const cave_talk_Encoder *const encoder_wheel_1, const cave_talk_Encoder *const encoder_wheel_2, const cave_talk_Encoder *const encoder_wheel_3)
{

    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t       ostream          = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_Odometry odometry_message = cave_talk_Odometry_init_zero;

        if (IMU != NULL)
        {
            odometry_message.Imu     = *IMU;
            odometry_message.has_Imu = true;
        }

        if (encoder_wheel_0 != NULL)
        {
            odometry_message.encoder_wheel_0     = *encoder_wheel_0;
            odometry_message.has_encoder_wheel_0 = true;
        }

        if (encoder_wheel_1 != NULL)
        {
            odometry_message.encoder_wheel_1     = *encoder_wheel_1;
            odometry_message.has_encoder_wheel_1 = true;
        }

        if (encoder_wheel_2 != NULL)
        {
            odometry_message.encoder_wheel_2     = *encoder_wheel_2;
            odometry_message.has_encoder_wheel_2 = true;
        }

        if (encoder_wheel_3 != NULL)
        {
            odometry_message.encoder_wheel_3     = *encoder_wheel_3;
            odometry_message.has_encoder_wheel_3 = true;
        }

        if (!pb_encode(&ostream, cave_talk_Odometry_fields, &odometry_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_ODOMETRY, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakLog(const CaveTalk_Handle_t *const handle, char *log)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        cave_talk_Log log_message = cave_talk_Log_init_zero;
        log_message.log_string.arg          = log;
        log_message.log_string.funcs.encode = CaveTalk_EncodeString;

        if (!pb_encode(&ostream, cave_talk_Log_fields, &log_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_LOG, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakConfigServoWheels(const CaveTalk_Handle_t *const handle, const cave_talk_Servo *const servo_wheel_0, const cave_talk_Servo *const servo_wheel_1, const cave_talk_Servo *const servo_wheel_2, const cave_talk_Servo *const servo_wheel_3)

{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {

        pb_ostream_t                ostream                     = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_ConfigServoWheels config_servo_wheels_message = cave_talk_ConfigServoWheels_init_zero;

        if (servo_wheel_0 != NULL)
        {
            config_servo_wheels_message.servo_wheel_0     = *servo_wheel_0;
            config_servo_wheels_message.has_servo_wheel_0 = true;
        }

        if (servo_wheel_1 != NULL)
        {
            config_servo_wheels_message.servo_wheel_1     = *servo_wheel_1;
            config_servo_wheels_message.has_servo_wheel_1 = true;
        }

        if (servo_wheel_2 != NULL)
        {
            config_servo_wheels_message.servo_wheel_2     = *servo_wheel_2;
            config_servo_wheels_message.has_servo_wheel_2 = true;
        }

        if (servo_wheel_3 != NULL)
        {
            config_servo_wheels_message.servo_wheel_3     = *servo_wheel_3;
            config_servo_wheels_message.has_servo_wheel_3 = true;
        }

        if (!pb_encode(&ostream, cave_talk_ConfigServoWheels_fields, &config_servo_wheels_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_CONFIG_SERVO_WHEELS, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakConfigServoCams(const CaveTalk_Handle_t *const handle, const cave_talk_Servo *const servo_cam_pan, const cave_talk_Servo *const servo_cam_tilt)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t              ostream                   = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_ConfigServoCams config_servo_cams_message = cave_talk_ConfigServoCams_init_zero;

        if (servo_cam_pan != NULL)
        {
            config_servo_cams_message.servo_cam_pan     = *servo_cam_pan;
            config_servo_cams_message.has_servo_cam_pan = true;
        }

        if (servo_cam_tilt != NULL)
        {
            config_servo_cams_message.servo_cam_tilt     = *servo_cam_tilt;
            config_servo_cams_message.has_servo_cam_tilt = true;
        }

        if (!pb_encode(&ostream, cave_talk_ConfigServoCams_fields, &config_servo_cams_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_CONFIG_SERVO_CAMS, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakConfigMotors(const CaveTalk_Handle_t *const handle, const cave_talk_Motor *const motor_wheel_0, const cave_talk_Motor *const motor_wheel_1, const cave_talk_Motor *const motor_wheel_2, const cave_talk_Motor *const motor_wheel_3)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {

        pb_ostream_t          ostream               = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_ConfigMotor config_motors_message = cave_talk_ConfigMotor_init_zero;

        if (motor_wheel_0 != NULL)
        {
            config_motors_message.motor_wheel_0     = *motor_wheel_0;
            config_motors_message.has_motor_wheel_0 = true;
        }

        if (motor_wheel_1 != NULL)
        {
            config_motors_message.motor_wheel_1     = *motor_wheel_1;
            config_motors_message.has_motor_wheel_1 = true;
        }

        if (motor_wheel_2 != NULL)
        {
            config_motors_message.motor_wheel_2     = *motor_wheel_2;
            config_motors_message.has_motor_wheel_2 = true;
        }

        if (motor_wheel_3 != NULL)
        {
            config_motors_message.motor_wheel_3     = *motor_wheel_3;
            config_motors_message.has_motor_wheel_3 = true;
        }

        if (!pb_encode(&ostream, cave_talk_ConfigMotor_fields, &config_motors_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_CONFIG_MOTOR, handle->buffer, ostream.bytes_written);
        }
    }
    return error;
}

CaveTalk_Error_t CaveTalk_SpeakConfigEncoders(const CaveTalk_Handle_t *const handle, const cave_talk_ConfigEncoder *const encoder_wheel_0, const cave_talk_ConfigEncoder *const encoder_wheel_1, const cave_talk_ConfigEncoder *const encoder_wheel_2, const cave_talk_ConfigEncoder *const encoder_wheel_3)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {

        pb_ostream_t             ostream                = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_ConfigEncoders config_encoder_message = cave_talk_ConfigEncoders_init_zero;

        if (encoder_wheel_0 != NULL)
        {
            config_encoder_message.encoder_wheel_0     = *encoder_wheel_0;
            config_encoder_message.has_encoder_wheel_0 = true;
        }

        if (encoder_wheel_1 != NULL)
        {
            config_encoder_message.encoder_wheel_1     = *encoder_wheel_1;
            config_encoder_message.has_encoder_wheel_1 = true;
        }

        if (encoder_wheel_2 != NULL)
        {
            config_encoder_message.encoder_wheel_2     = *encoder_wheel_2;
            config_encoder_message.has_encoder_wheel_2 = true;
        }

        if (encoder_wheel_3 != NULL)
        {
            config_encoder_message.encoder_wheel_3     = *encoder_wheel_3;
            config_encoder_message.has_encoder_wheel_3 = true;
        }

        if (!pb_encode(&ostream, cave_talk_ConfigEncoders_fields, &config_encoder_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_CONFIG_ENCODER, handle->buffer, ostream.bytes_written);
        }
    }
    return error;
}

CaveTalk_Error_t CaveTalk_SpeakConfigLog(const CaveTalk_Handle_t *const handle, const cave_talk_LogLevel log_level)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t        ostream            = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_ConfigLog config_log_message = cave_talk_ConfigLog_init_zero;

        config_log_message.log_level = log_level;

        if (!pb_encode(&ostream, cave_talk_ConfigLog_fields, &config_log_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_CONFIG_LOG, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

CaveTalk_Error_t CaveTalk_SpeakConfigWheelSpeedControl(const CaveTalk_Handle_t *const handle, const cave_talk_PID *const wheel_0_params, const cave_talk_PID *const wheel_1_params, const cave_talk_PID *const wheel_2_params, const cave_talk_PID *const wheel_3_params, const bool enabled)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {

        pb_ostream_t                      ostream            = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_ConfigWheelSpeedControl config_wsc_message = cave_talk_ConfigWheelSpeedControl_init_zero;

        if (wheel_0_params != NULL)
        {
            config_wsc_message.wheel_0_params     = *wheel_0_params;
            config_wsc_message.has_wheel_0_params = true;
        }

        if (wheel_1_params != NULL)
        {
            config_wsc_message.wheel_1_params     = *wheel_1_params;
            config_wsc_message.has_wheel_1_params = true;
        }

        if (wheel_2_params != NULL)
        {
            config_wsc_message.wheel_2_params     = *wheel_2_params;
            config_wsc_message.has_wheel_2_params = true;
        }

        if (wheel_3_params != NULL)
        {
            config_wsc_message.wheel_3_params     = *wheel_3_params;
            config_wsc_message.has_wheel_3_params = true;
        }

        config_wsc_message.enabled = enabled;

        if (!pb_encode(&ostream, cave_talk_ConfigWheelSpeedControl_fields, &config_wsc_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_CONFIG_WHEEL_SPEED_CONTROL, handle->buffer, ostream.bytes_written);
        }
    }
    return error;
}

CaveTalk_Error_t CaveTalk_SpeakConfigSteeringControl(const CaveTalk_Handle_t *const handle, const cave_talk_PID *const turn_rate_params, const bool enabled)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {

        pb_ostream_t                    ostream           = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_ConfigSteeringControl config_sc_message = cave_talk_ConfigSteeringControl_init_zero;

        if (turn_rate_params != NULL)
        {
            config_sc_message.turn_rate_params     = *turn_rate_params;
            config_sc_message.has_turn_rate_params = true;
        }

        config_sc_message.enabled = enabled;

        if (!pb_encode(&ostream, cave_talk_ConfigSteeringControl_fields, &config_sc_message))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_CONFIG_STEERING_CONTROL, handle->buffer, ostream.bytes_written);
        }
    }
    return error;
}

CaveTalk_Error_t CaveTalk_SpeakAirQuality(const CaveTalk_Handle_t *const handle, const uint32_t dust_ug_per_m3, const uint32_t gas_ppm, const double temperature_celsius)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NULL;

    if ((NULL == handle) || (NULL == handle->buffer) || (NULL == handle->link_handle.send))
    {
    }
    else
    {
        pb_ostream_t         ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cave_talk_AirQuality aq_msg  = cave_talk_AirQuality_init_zero;

        aq_msg.dust_ug_per_m3      = dust_ug_per_m3;
        aq_msg.gas_ppm             = gas_ppm;
        aq_msg.temperature_celsius = temperature_celsius;

        if (!pb_encode(&ostream, cave_talk_AirQuality_fields, &aq_msg))
        {
            error = CAVE_TALK_ERROR_SIZE;
        }
        else
        {
            error = CaveTalk_Speak(&handle->link_handle, (CaveTalk_Id_t)cave_talk_Id_ID_AIR_QUALITY, handle->buffer, ostream.bytes_written);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleOogaBooga(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t        istream            = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_OogaBooga ooga_booga_message = cave_talk_OogaBooga_init_zero;

        if (!pb_decode(&istream, cave_talk_OogaBooga_fields, &ooga_booga_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_ooga_booga)
        {
            handle->listen_callbacks.hear_ooga_booga(ooga_booga_message.ooga_booga);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleMovement(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t       istream          = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_Movement movement_message = cave_talk_Movement_init_zero;

        if (!pb_decode(&istream, cave_talk_Movement_fields, &movement_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_movement)
        {
            handle->listen_callbacks.hear_movement(movement_message.speed_meters_per_second, movement_message.turn_rate_radians_per_second);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleCameraMovement(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t             istream                 = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_CameraMovement camera_movement_message = cave_talk_CameraMovement_init_zero;

        if (!pb_decode(&istream, cave_talk_CameraMovement_fields, &camera_movement_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_camera_movement)
        {
            handle->listen_callbacks.hear_camera_movement(camera_movement_message.pan_angle_radians, camera_movement_message.tilt_angle_radians);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleLights(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t     istream        = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_Lights lights_message = cave_talk_Lights_init_zero;

        if (!pb_decode(&istream, cave_talk_Lights_fields, &lights_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_lights)
        {
            handle->listen_callbacks.hear_lights(lights_message.headlights);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleArm(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t  istream     = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_Arm arm_message = cave_talk_Arm_init_zero;

        if (!pb_decode(&istream, cave_talk_Arm_fields, &arm_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_arm)
        {
            handle->listen_callbacks.hear_arm(arm_message.arm);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleOdometry(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t       istream          = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_Odometry odometry_message = cave_talk_Odometry_init_zero;

        if (!pb_decode(&istream, cave_talk_Odometry_fields, &odometry_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_odometry)
        {
            handle->listen_callbacks.hear_odometry(&odometry_message.Imu, &odometry_message.encoder_wheel_0, &odometry_message.encoder_wheel_1, &odometry_message.encoder_wheel_2, &odometry_message.encoder_wheel_3);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleLog(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t  istream     = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_Log log_message = cave_talk_Log_init_zero;

        log_message.log_string.funcs.decode = CaveTalk_DecodeString;

        if ((!pb_decode(&istream, cave_talk_Log_fields, &log_message)) || (NULL == log_message.log_string.arg))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_log)
        {
            handle->listen_callbacks.hear_log((const char *const)log_message.log_string.arg);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleConfigServoWheels(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)

{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {

        pb_istream_t                istream                     = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_ConfigServoWheels config_servo_wheels_message = cave_talk_ConfigServoWheels_init_zero;

        if (!pb_decode(&istream, cave_talk_ConfigServoWheels_fields, &config_servo_wheels_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_config_servo_wheels)
        {
            handle->listen_callbacks.hear_config_servo_wheels(&config_servo_wheels_message.servo_wheel_0, &config_servo_wheels_message.servo_wheel_1, &config_servo_wheels_message.servo_wheel_2, &config_servo_wheels_message.servo_wheel_3);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleConfigServoCams(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t              istream                   = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_ConfigServoCams config_servo_cams_message = cave_talk_ConfigServoCams_init_zero;

        if (!pb_decode(&istream, cave_talk_ConfigServoCams_fields, &config_servo_cams_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_config_servo_cams)
        {
            handle->listen_callbacks.hear_config_servo_cams(&config_servo_cams_message.servo_cam_pan, &config_servo_cams_message.servo_cam_tilt);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleConfigMotor(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t          istream              = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_ConfigMotor config_motor_message = cave_talk_ConfigMotor_init_zero;

        if (!pb_decode(&istream, cave_talk_ConfigMotor_fields, &config_motor_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_config_motors)
        {
            handle->listen_callbacks.hear_config_motors(&config_motor_message.motor_wheel_0, &config_motor_message.motor_wheel_1, &config_motor_message.motor_wheel_2, &config_motor_message.motor_wheel_3);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleConfigEncoders(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t             istream                = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_ConfigEncoders config_encoder_message = cave_talk_ConfigEncoders_init_zero;

        if (!pb_decode(&istream, cave_talk_ConfigEncoders_fields, &config_encoder_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_config_encoders)
        {
            handle->listen_callbacks.hear_config_encoders(&config_encoder_message.encoder_wheel_0, &config_encoder_message.encoder_wheel_1, &config_encoder_message.encoder_wheel_2, &config_encoder_message.encoder_wheel_3);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleConfigLog(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t        istream            = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_ConfigLog config_log_message = cave_talk_ConfigLog_init_zero;

        if (!pb_decode(&istream, cave_talk_ConfigLog_fields, &config_log_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_config_log)
        {
            handle->listen_callbacks.hear_config_log(config_log_message.log_level);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleConfigWheelSpeedControl(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t                      istream            = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_ConfigWheelSpeedControl config_wsc_message = cave_talk_ConfigWheelSpeedControl_init_zero;

        if (!pb_decode(&istream, cave_talk_ConfigWheelSpeedControl_fields, &config_wsc_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_config_wheel_speed_control)
        {
            handle->listen_callbacks.hear_config_wheel_speed_control(&config_wsc_message.wheel_0_params, &config_wsc_message.wheel_1_params, &config_wsc_message.wheel_2_params, &config_wsc_message.wheel_3_params, config_wsc_message.enabled);
        }
    }

    return error;
}


static CaveTalk_Error_t CaveTalk_HandleConfigSteeringControl(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t                    istream           = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_ConfigSteeringControl config_sc_message = cave_talk_ConfigSteeringControl_init_zero;

        if (!pb_decode(&istream, cave_talk_ConfigSteeringControl_fields, &config_sc_message))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_config_steering_control)
        {
            handle->listen_callbacks.hear_config_steering_control(&config_sc_message.turn_rate_params, config_sc_message.enabled);
        }
    }

    return error;
}

static CaveTalk_Error_t CaveTalk_HandleAirQuality(const CaveTalk_Handle_t *const handle, const CaveTalk_Length_t length)
{
    CaveTalk_Error_t error = CAVE_TALK_ERROR_NONE;

    if ((NULL == handle) || (NULL == handle->buffer))
    {
        error = CAVE_TALK_ERROR_NULL;
    }
    else
    {
        pb_istream_t         istream = pb_istream_from_buffer(handle->buffer, length);
        cave_talk_AirQuality aq_msg  = cave_talk_AirQuality_init_zero;

        if (!pb_decode(&istream, cave_talk_AirQuality_fields, &aq_msg))
        {
            error = CAVE_TALK_ERROR_PARSE;
        }
        else if (NULL != handle->listen_callbacks.hear_air_quality)
        {
            handle->listen_callbacks.hear_air_quality(aq_msg.dust_ug_per_m3, aq_msg.gas_ppm, aq_msg.temperature_celsius);
        }
    }

    return error;
}

static bool CaveTalk_EncodeString(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    bool        encoded = false;
    const char *string  = (const char *)(*arg);

    if (pb_encode_tag_for_field(stream, field))
    {
        encoded = pb_encode_string(stream, (uint8_t *)string, strlen(string));
    }

    return encoded;
}

static bool CaveTalk_DecodeString(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    CAVE_TALK_UNUSED(field);

    bool decoded = false;

    if (NULL == arg)
    {
    }
    else
    {
        static char log_buffer[CAVE_TALK_MAX_PAYLOAD_SIZE] = {
            0U
        };

        *arg = NULL;

        if ((stream->bytes_left <= (sizeof(log_buffer) - 1)) && (pb_read(stream, (unsigned char *)log_buffer, stream->bytes_left)))
        {
            *arg    = (void *)log_buffer;
            decoded = true;
        }
    }

    return decoded;
}