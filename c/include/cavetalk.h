#ifndef CAVETALK_H
#define CAVETALK_H

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

typedef uint32_t CaveTalk_Id_t;
typedef uint8_t  CaveTalk_Length_t;

typedef struct
{
    char *key;
    uint8_t *data;
    size_t size;
} CaveTalk_Message_t;

typedef struct
{
    void (*hear_log)(const char *const log);
    void (*hear_arm)(const cavetalk_Mode mode);
    void (*hear_drive)(const cavetalk_Drive *const drive);
    void (*hear_acceleration)(const cavetalk_Acceleration *const acceleration);
    void (*hear_gyroscope)(const cavetalk_Gyroscope *const gyroscope);
    void (*hear_encoders)(const cavetalk_Encoders *const encoders);
} CaveTalk_Callbacks_t;

typedef struct
{
    CaveTalk_Callbacks_t callbacks;
    CaveTalk_Id_t id;
    uint8_t *buffer;
    size_t buffer_size;
} CaveTalk_Handle_t;

static const CaveTalk_Message_t kCaveTalk_MessageNull = {
    .key  = NULL,
    .data = NULL,
    .size = 0U,
};

static const CaveTalk_Callbacks_t kCaveTalk_CallbacksNull = {
    .hear_log          = NULL,
    .hear_arm          = NULL,
    .hear_drive        = NULL,
    .hear_acceleration = NULL,
    .hear_gyroscope    = NULL,
    .hear_encoders     = NULL,
};

static const CaveTalk_Handle_t kCaveTalk_HandleNull = {
    .callbacks   = kCaveTalk_CallbacksNull,
    .id          = 0U,
    .buffer      = NULL,
    .buffer_size = 0U,
};

#ifdef __cplusplus
extern "C"
{
#endif

bool CaveTalk_Initialize(CaveTalk_Handle_t *const handle, const CaveTalk_Callbacks_t *const callbacks, const CaveTalk_Id_t id, uint8_t *const buffer, const size_t buffer_size);
bool CaveTalk_IsMessageValid(const CaveTalk_Message_t *const message);
void CaveTalk_Hear(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t message);
CaveTalk_Message_t CaveTalk_SpeakLog(const CaveTalk_Handle_t *const handle, char *const log);
CaveTalk_Message_t CaveTalk_SpeakArm(const CaveTalk_Handle_t *const handle, const cavetalk_Mode mode);
CaveTalk_Message_t CaveTalk_SpeakDrive(const CaveTalk_Handle_t *const handle, const cavetalk_Drive *const drive);
CaveTalk_Message_t CaveTalk_SpeakAcceleration(const CaveTalk_Handle_t *const handle, const cavetalk_Acceleration *const acceleration);
CaveTalk_Message_t CaveTalk_SpeakGyroscope(const CaveTalk_Handle_t *const handle, const cavetalk_Gyroscope *const gyroscope);
CaveTalk_Message_t CaveTalk_SpeakEncoders(const CaveTalk_Handle_t *const handle, const cavetalk_Encoders *const encoders);

#ifdef __cplusplus
}
#endif

#endif /* CAVETALK_H */
