#include "cavetalk.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
#define CAVETALK_MAX_PAYLOAD_SIZE 256U

typedef struct
{
    cavetalk_Encoder *encoders;
    size_t count;
} CaveTalk_EncoderList_t;

const CaveTalk_Message_t   kCaveTalk_MessageNull = {
    .key  = NULL,
    .data = NULL,
    .size = 0U,
};
const CaveTalk_Callbacks_t kCaveTalk_CallbacksNull = {
    .hear_log          = NULL,
    .hear_arm          = NULL,
    .hear_drive        = NULL,
    .hear_acceleration = NULL,
    .hear_gyroscope    = NULL,
    .hear_encoders     = NULL,
};
const CaveTalk_Handle_t    kCaveTalk_HandleNull = {
    .callbacks   = kCaveTalk_CallbacksNull,
    .message     = kCaveTalk_MessageNull,
    .id          = 0U,
    .buffer      = NULL,
    .buffer_size = 0U,
};

static char              CaveTalk_DecodeBuffer[CAVETALK_MAX_PAYLOAD_SIZE];
static const char *const kCaveTalk_KeyDelimiter                  = "/";
static const char *const kCaveTalk_TopicKeys[cavetalk_Id_ID_MAX] = {
    [cavetalk_Id_ID_NONE]         = "none",
    [cavetalk_Id_ID_LOG]          = "log",
    [cavetalk_Id_ID_ARM]          = "arm",
    [cavetalk_Id_ID_DRIVE]        = "drive",
    [cavetalk_Id_ID_ACCELERATION] = "acceleration",
    [cavetalk_Id_ID_GYROSCOPE]    = "gyroscope",
    [cavetalk_Id_ID_ENCODERS]     = "encoders",
};

static cavetalk_Id CaveTalk_GetId(const CaveTalk_Handle_t *const handle, const char *const key);
static char *CaveTalk_BuildKey(const CaveTalk_Handle_t *const handle, const cavetalk_Id id);
static CaveTalk_Message_t *CaveTalk_Speak(CaveTalk_Handle_t *handle, const cavetalk_Id id, const size_t data_size);
static void CaveTalk_HandleLog(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleArm(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleDrive(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleAcceleration(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleGyroscope(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static void CaveTalk_HandleEncoders(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t *const message);
static bool CaveTalk_EncodeString(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
static bool CaveTalk_DecodeString(pb_istream_t *stream, const pb_field_t *field, void **arg);
static bool CaveTalk_EncodeRepeatedSubmessage(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
static bool CaveTalk_DecodeRepeatedSubmessage(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool CaveTalk_Initialize(CaveTalk_Handle_t *const handle, const CaveTalk_Callbacks_t *const callbacks, const CaveTalk_Id_t id, uint8_t *const buffer, const size_t buffer_size)
{
    bool initialized = false;

    if ((NULL != handle) && (NULL != callbacks) && (NULL != buffer) && (0U != buffer_size))
    {
        handle->callbacks   = *callbacks;
        handle->message     = kCaveTalk_MessageNull;
        handle->id          = id;
        handle->buffer      = buffer;
        handle->buffer_size = buffer_size;

        initialized = true;
    }

    return initialized;
}

char *CaveTalk_GetKey(CaveTalk_Handle_t *const handle, const CaveTalk_Id_t peer_id, const cavetalk_Id key_id)
{
    char *key = NULL;

    if (NULL != handle)
    {
        const CaveTalk_Id_t id = handle->id;
        handle->id      = peer_id;
        handle->message = kCaveTalk_MessageNull;

        key        = CaveTalk_BuildKey(handle, key_id);
        handle->id = id;
    }

    return key;
}

void CaveTalk_Hear(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t message)
{
    if ((NULL != handle) && (NULL != message.key) && (NULL != message.data) && (0U != message.size))
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

CaveTalk_Message_t *CaveTalk_SpeakLog(CaveTalk_Handle_t *const handle, char *const log)
{
    CaveTalk_Message_t *message = NULL;

    if ((NULL != handle) && (NULL != log))
    {
        pb_ostream_t ostream     = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cavetalk_Log log_message = cavetalk_Log_init_zero;

        log_message.log_string.arg          = log;
        log_message.log_string.funcs.encode = CaveTalk_EncodeString;

        if (pb_encode(&ostream, cavetalk_Log_fields, &log_message))
        {
            message = CaveTalk_Speak(handle, cavetalk_Id_ID_LOG, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t *CaveTalk_SpeakArm(CaveTalk_Handle_t *const handle, const cavetalk_Mode mode)
{
    CaveTalk_Message_t *message = NULL;

    if (NULL != handle)
    {
        pb_ostream_t ostream     = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cavetalk_Arm arm_message = cavetalk_Arm_init_zero;

        arm_message.mode = mode;

        if (pb_encode(&ostream, cavetalk_Arm_fields, &arm_message))
        {
            message = CaveTalk_Speak(handle, cavetalk_Id_ID_ARM, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t *CaveTalk_SpeakDrive(CaveTalk_Handle_t *const handle, const cavetalk_Drive *const drive)
{
    CaveTalk_Message_t *message = NULL;

    if ((NULL != handle) && (NULL != drive))
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        if (pb_encode(&ostream, cavetalk_Drive_fields, drive))
        {
            message = CaveTalk_Speak(handle, cavetalk_Id_ID_DRIVE, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t *CaveTalk_SpeakAcceleration(CaveTalk_Handle_t *const handle, const cavetalk_Acceleration *const acceleration)
{
    CaveTalk_Message_t *message = NULL;

    if ((NULL != handle) && (NULL != acceleration))
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        if (pb_encode(&ostream, cavetalk_Acceleration_fields, acceleration))
        {
            message = CaveTalk_Speak(handle, cavetalk_Id_ID_ACCELERATION, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t *CaveTalk_SpeakGyroscope(CaveTalk_Handle_t *const handle, const cavetalk_Gyroscope *const gyroscope)
{
    CaveTalk_Message_t *message = NULL;

    if ((NULL != handle) && (NULL != gyroscope))
    {
        pb_ostream_t ostream = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);

        if (pb_encode(&ostream, cavetalk_Gyroscope_fields, gyroscope))
        {
            message = CaveTalk_Speak(handle, cavetalk_Id_ID_GYROSCOPE, ostream.bytes_written);
        }
    }

    return message;
}

CaveTalk_Message_t *CaveTalk_SpeakEncoders(CaveTalk_Handle_t *const handle, cavetalk_Encoder *const encoders, const size_t count)
{
    CaveTalk_Message_t *message = NULL;

    if ((NULL != handle) && (NULL != encoders) && (count > 0U))
    {
        pb_ostream_t           ostream          = pb_ostream_from_buffer(handle->buffer, handle->buffer_size);
        cavetalk_Encoders      encoders_message = cavetalk_Encoders_init_zero;
        CaveTalk_EncoderList_t encoder_list     = {
            .encoders = encoders,
            .count    = count,
        };

        encoders_message.encoders.arg          = &encoder_list;
        encoders_message.encoders.funcs.encode = CaveTalk_EncodeRepeatedSubmessage;

        if (pb_encode(&ostream, cavetalk_Encoders_fields, &encoders_message))
        {
            message = CaveTalk_Speak(handle, cavetalk_Id_ID_ENCODERS, ostream.bytes_written);
        }
    }

    return message;
}

static cavetalk_Id CaveTalk_GetId(const CaveTalk_Handle_t *const handle, const char *const key)
{
    cavetalk_Id  id     = cavetalk_Id_ID_NONE;
    const size_t length = (size_t)strnlen(key, handle->buffer_size);

    if (length < handle->buffer_size)
    {
        const char *topic = memchr(key, *kCaveTalk_KeyDelimiter, length);

        if (NULL != topic)
        {
            ++topic;
            const size_t topic_length = length - (size_t)(topic - key);

            if (topic_length > 0U)
            {
                for (size_t i = 0U; i < (sizeof(kCaveTalk_TopicKeys) / sizeof(kCaveTalk_TopicKeys[0])); i++)
                {
                    if (0 == strncmp(kCaveTalk_TopicKeys[i], topic, topic_length))
                    {
                        id = (cavetalk_Id)i;
                        break;
                    }
                }
            }
        }
    }

    return id;
}

static char *CaveTalk_BuildKey(const CaveTalk_Handle_t *const handle, const cavetalk_Id id)
{
    char *       key      = NULL;
    const size_t max_size = handle->buffer_size - handle->message.size;

    if ((max_size > 0U) && (cavetalk_Id_ID_NONE != id))
    {
        char *const string_buffer = (char *)(handle->buffer + handle->message.size);
        const int   size          = snprintf(string_buffer, max_size, "%lu%s", (unsigned long)handle->id, kCaveTalk_KeyDelimiter);

        if ((size > 0) && ((size_t)size < max_size))
        {
            const size_t size_remaining = max_size - (size_t)size;

            if (strnlen(kCaveTalk_TopicKeys[id], size_remaining) < size_remaining)
            {
                (void)strncat(string_buffer, kCaveTalk_TopicKeys[id], size_remaining);
                key = string_buffer;
            }
        }
    }

    return key;
}

static CaveTalk_Message_t *CaveTalk_Speak(CaveTalk_Handle_t *const handle, const cavetalk_Id id, const size_t data_size)
{
    CaveTalk_Message_t *message = NULL;

    handle->message.data = handle->buffer;
    handle->message.size = data_size;
    handle->message.key  = CaveTalk_BuildKey(handle, id);

    if (NULL != handle->message.key)
    {
        message = &handle->message;
    }
    else
    {
        handle->message = kCaveTalk_MessageNull;
    }

    return message;
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
    pb_istream_t      istream          = pb_istream_from_buffer(message->data, message->size);
    cavetalk_Encoders encoders_message = cavetalk_Encoders_init_zero;
    size_t            count            = 0U;

    encoders_message.encoders.arg          = &count;
    encoders_message.encoders.funcs.decode = CaveTalk_DecodeRepeatedSubmessage;

    if ((NULL != handle->callbacks.hear_encoders) && pb_decode(&istream, cavetalk_Encoders_fields, &encoders_message))
    {
        handle->callbacks.hear_encoders((const cavetalk_Encoder *const)CaveTalk_DecodeBuffer, count);
    }
}

static bool CaveTalk_EncodeString(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    bool              encoded = false;
    const char *const string  = (const char *const)(*arg);

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
        const size_t bytes_read = stream->bytes_left;
        *arg = NULL;

        if ((stream->bytes_left <= (sizeof(CaveTalk_DecodeBuffer) - 1U)) && (pb_read(stream, (unsigned char *)CaveTalk_DecodeBuffer, stream->bytes_left)))
        {
            CaveTalk_DecodeBuffer[bytes_read] = '\0';
            *arg                              = (void *)CaveTalk_DecodeBuffer;
            decoded                           = true;
        }
    }

    return decoded;
}

static bool CaveTalk_EncodeRepeatedSubmessage(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    bool                                encoded      = true;
    const CaveTalk_EncoderList_t *const encoder_list = (const CaveTalk_EncoderList_t *const)*arg;

    for (size_t i = 0U; i < encoder_list->count; i++)
    {
        if (!pb_encode_tag_for_field(stream, field) || !pb_encode_submessage(stream, cavetalk_Encoder_fields, &encoder_list->encoders[i]))
        {
            encoded = false;
            break;
        }
    }

    return encoded;
}

static bool CaveTalk_DecodeRepeatedSubmessage(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    CAVETALK_UNUSED(field);

    bool          decoded = false;
    size_t *const count   = (size_t *)*arg;
    const size_t  offset  = *count * sizeof(cavetalk_Encoder);

    if ((sizeof(CaveTalk_DecodeBuffer) - offset) >= sizeof(cavetalk_Encoder))
    {
        cavetalk_Encoder *const encoder = (cavetalk_Encoder *)(CaveTalk_DecodeBuffer + offset);
        *encoder = (cavetalk_Encoder)cavetalk_Encoder_init_zero;

        if (pb_decode(stream, cavetalk_Encoder_fields, encoder))
        {
            decoded = true;
            ++*count;
        }
    }

    return decoded;
}
