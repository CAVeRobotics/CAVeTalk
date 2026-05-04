#include "cavetalk.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "acceleration.pb.h"
#include "arm.pb.h"
#include "drive.pb.h"
#include "encoders.pb.h"
#include "gyroscope.pb.h"
#include "ids.pb.h"
#include "log.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

#define CAVETALK_UNUSED(arg)      (void)(arg)
#define CAVETALK_MAX_PAYLOAD_SIZE 255U

static char CaveTalk_DecodeBuffer[CAVETALK_MAX_PAYLOAD_SIZE];

static cavetalk_Id CaveTalk_GetId(const CaveTalk_Handle_t *const handle, const char *const key);
static char *CaveTalk_GetKey(const CaveTalk_Handle_t *const handle, const cavetalk_Id id);
static void CaveTalk_Speak(const CaveTalk_Handle_t *const handle, CaveTalk_Message_t *const message, const cavetalk_Id id, const size_t size);
static void CaveTalk_HandleLog(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleArm(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleDrive(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleAcceleration(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleGyroscope(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleEncoders(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static bool CaveTalk_EncodeString(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
static bool CaveTalk_DecodeString(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool CaveTalk_Initialize(CaveTalk_Handle_t *const handle, const CaveTalk_Callbacks_t *const callbacks, const CaveTalk_Id_t id, uint8_t *const buffer, const size_t buffer_size)
{
    bool initialized = false;

    if ((NULL != handle) && (NULL != callbacks) && (NULL != buffer) && (0U != buffer_size))
    {
        handle->callbacks   = *callbacks;
        handle->callbacks   = *callbacks;
        handle->id          = id;
        handle->buffer      = buffer;
        handle->buffer_size = buffer_size;

        initialized = true;
    }

    return initialized;
}

bool CaveTalk_IsMessageValid(const CaveTalk_Message_t *const message)
{
    bool valid = false;

    if ((NULL != message) && (NULL != message->key) && (NULL != message->data) && (0U != message->size))
    {
        valid = true;
    }

    return valid;
}

void CaveTalk_Hear(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t message)
{
    if ((NULL != handle) && CaveTalk_IsMessageValid(&message))
    {
        switch (CaveTalk_GetId(handle, message.key))
        {
        case cavetalk_Id_ID_LOG:
            CaveTalk_HandleLog(handle, &message);
            break;
        case cavetalk_Id_ID_ARM:
            CaveTalk_HandleArm(handle, &message);
            break;
        case cavetalk_Id_ID_DRIVE:
            CaveTalk_HandleDrive(handle, &message);
            break;
        case cavetalk_Id_ID_ACCELERATION:
            CaveTalk_HandleAcceleration(handle, &message);
            break;
        case cavetalk_Id_ID_GYROSCOPE:
            CaveTalk_HandleGyroscope(handle, &message);
            break;
        case cavetalk_Id_ID_ENCODERS:
            CaveTalk_HandleEncoders(handle, &message);
            break;
        case cavetalk_Id_ID_NONE:
        default:
            break;
        }
    }
}

CaveTalk_Message_t CaveTalk_SpeakLog(const CaveTalk_Handle_t *const handle, char *const log)
{
    CaveTalk_Message_t message = kCaveTalk_MessageNull;

    if ((NULL != handle) && (NULL != log))
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        cavetalk_Log log_message = cavetalk_Log_init_zero;
        log_message.log_string.arg          = log;
        log_message.log_string.funcs.encode = CaveTalk_EncodeString;

        if (pb_encode(&ostream, cavetalk_Log_fields, &log_message))
        {
            CaveTalk_Speak(handle, &message, cavetalk_Id_ID_LOG, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t CaveTalk_SpeakArm(const CaveTalk_Handle_t *const handle, const cavetalk_Mode mode)
{
    CaveTalk_Message_t message = kCaveTalk_MessageNull;

    if (NULL != handle)
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        cavetalk_Arm arm_message = cavetalk_Arm_init_zero;

        arm_message.mode = mode;

        if (pb_encode(&ostream, cavetalk_Arm_fields, &arm_message))
        {
            CaveTalk_Speak(handle, &message, cavetalk_Id_ID_ARM, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t CaveTalk_SpeakDrive(const CaveTalk_Handle_t *const handle, const cavetalk_Drive *const drive)
{
    CaveTalk_Message_t message = kCaveTalk_MessageNull;

    if ((NULL != handle) && (NULL != drive))
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        if (pb_encode(&ostream, cavetalk_Drive_fields, drive))
        {
            CaveTalk_Speak(handle, &message, cavetalk_Id_ID_DRIVE, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t CaveTalk_SpeakAcceleration(const CaveTalk_Handle_t *const handle, const cavetalk_Acceleration *const acceleration)
{
    CaveTalk_Message_t message = kCaveTalk_MessageNull;

    if ((NULL != handle) && (NULL != acceleration))
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        if (pb_encode(&ostream, cavetalk_Acceleration_fields, acceleration))
        {
            CaveTalk_Speak(handle, &message, cavetalk_Id_ID_ACCELERATION, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t CaveTalk_SpeakGyroscope(const CaveTalk_Handle_t *const handle, const cavetalk_Gyroscope *const gyroscope)
{
    CaveTalk_Message_t message = kCaveTalk_MessageNull;

    if ((NULL != handle) && (NULL != gyroscope))
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        if (pb_encode(&ostream, cavetalk_Gyroscope_fields, gyroscope))
        {
            CaveTalk_Speak(handle, &message, cavetalk_Id_ID_GYROSCOPE, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t CaveTalk_SpeakEncoders(const CaveTalk_Handle_t *const handle, const cavetalk_Encoders *const encoders)
{
    CaveTalk_Message_t message = kCaveTalk_MessageNull;

    if ((NULL != handle) && (NULL != encoders))
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        /* TODO encoder multiple fields */

        if (pb_encode(&ostream, cavetalk_Encoders_fields, encoders))
        {
            CaveTalk_Speak(handle, &message, cavetalk_Id_ID_ENCODERS, ostream.bytes_written);
        }
    }

    return message;
}

static cavetalk_Id CaveTalk_GetId(const CaveTalk_Handle_t *const handle, const char *const key)
{
    CAVETALK_UNUSED(handle);
    CAVETALK_UNUSED(key);

    cavetalk_Id id = cavetalk_Id_ID_NONE;

    /* TODO */

    return id;
}

static char *CaveTalk_GetKey(const CaveTalk_Handle_t *const handle, const cavetalk_Id id)
{
    CAVETALK_UNUSED(handle);
    CAVETALK_UNUSED(id);

    /* TODO */

    return NULL;
}

static void CaveTalk_Speak(const CaveTalk_Handle_t *const handle, CaveTalk_Message_t *const message, const cavetalk_Id id, const size_t size)
{
    message->key  = CaveTalk_GetKey(handle, id);
    message->data = handle->buffer;
    message->size = size;
}

static void CaveTalk_HandleLog(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message)
{
    pb_istream_t istream     = pb_istream_from_buffer(message->data, message->size);
    cavetalk_Log log_message = cavetalk_Log_init_zero;

    log_message.log_string.funcs.decode = CaveTalk_DecodeString;

    if ((NULL != handle->callbacks.hear_log) && pb_decode(&istream, cavetalk_Log_fields, &log_message) && (NULL != log_message.log_string.arg))
    {
        handle->callbacks.hear_log((const char *const)log_message.log_string.arg);
    }
}

static void CaveTalk_HandleArm(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message)
{
    pb_istream_t istream     = pb_istream_from_buffer(message->data, message->size);
    cavetalk_Arm arm_message = cavetalk_Arm_init_zero;

    if ((NULL != handle->callbacks.hear_arm) && pb_decode(&istream, cavetalk_Arm_fields, &arm_message))
    {
        handle->callbacks.hear_arm(arm_message.mode);
    }
}

static void CaveTalk_HandleDrive(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message)
{
    pb_istream_t   istream       = pb_istream_from_buffer(message->data, message->size);
    cavetalk_Drive drive_message = cavetalk_Drive_init_zero;

    if ((NULL != handle->callbacks.hear_drive) && pb_decode(&istream, cavetalk_Drive_fields, &drive_message))
    {
        handle->callbacks.hear_drive(&drive_message);
    }
}

static void CaveTalk_HandleAcceleration(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message)
{
    pb_istream_t          istream              = pb_istream_from_buffer(message->data, message->size);
    cavetalk_Acceleration acceleration_message = cavetalk_Acceleration_init_zero;

    if ((NULL != handle->callbacks.hear_acceleration) && pb_decode(&istream, cavetalk_Acceleration_fields, &acceleration_message))
    {
        handle->callbacks.hear_acceleration(&acceleration_message);
    }
}

static void CaveTalk_HandleGyroscope(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message)
{
    pb_istream_t       istream           = pb_istream_from_buffer(message->data, message->size);
    cavetalk_Gyroscope gyroscope_message = cavetalk_Gyroscope_init_zero;

    if ((NULL != handle->callbacks.hear_gyroscope) && pb_decode(&istream, cavetalk_Gyroscope_fields, &gyroscope_message))
    {
        handle->callbacks.hear_gyroscope(&gyroscope_message);
    }
}

static void CaveTalk_HandleEncoders(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message)
{
    CAVETALK_UNUSED(handle);
    CAVETALK_UNUSED(message);

    /* TODO repeated fields */
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
    CAVETALK_UNUSED(field);

    bool decoded = false;

    if (NULL != arg)
    {
        /* TODO use handle buffer? */

        *arg = NULL;

        if ((stream->bytes_left <= (sizeof(CaveTalk_DecodeBuffer) - 1)) && (pb_read(stream, (unsigned char *)CaveTalk_DecodeBuffer, stream->bytes_left)))
        {
            *arg    = (void *)CaveTalk_DecodeBuffer;
            decoded = true;
        }
    }

    return decoded;
}
