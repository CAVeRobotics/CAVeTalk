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
    void (*hear_encoders)(const cavetalk_Encoder *const encoders, const size_t count);
} CaveTalk_Callbacks_t;

typedef struct
{
    CaveTalk_Callbacks_t callbacks;
    CaveTalk_Message_t message;
    CaveTalk_Id_t id;
    uint8_t *buffer;
    size_t buffer_size;
} CaveTalk_Handle_t;

extern const CaveTalk_Message_t   kCaveTalk_MessageNull;
extern const CaveTalk_Callbacks_t kCaveTalk_CallbacksNull;
extern const CaveTalk_Handle_t    kCaveTalk_HandleNull;

#ifdef __cplusplus
extern "C"
{
#endif

bool CaveTalk_Initialize(CaveTalk_Handle_t *const handle, const CaveTalk_Callbacks_t *const callbacks, const CaveTalk_Id_t id, uint8_t *const buffer, const size_t buffer_size);
char *CaveTalk_GetKey(CaveTalk_Handle_t *const handle, const CaveTalk_Id_t peer_id, const cavetalk_Id key_id);
void CaveTalk_Hear(const CaveTalk_Handle_t *const handle, const CaveTalk_Message_t message);
CaveTalk_Message_t *CaveTalk_SpeakLog(CaveTalk_Handle_t *const handle, char *const log);
CaveTalk_Message_t *CaveTalk_SpeakArm(CaveTalk_Handle_t *const handle, const cavetalk_Mode mode);
CaveTalk_Message_t *CaveTalk_SpeakDrive(CaveTalk_Handle_t *const handle, const cavetalk_Drive *const drive);
CaveTalk_Message_t *CaveTalk_SpeakAcceleration(CaveTalk_Handle_t *const handle, const cavetalk_Acceleration *const acceleration);
CaveTalk_Message_t *CaveTalk_SpeakGyroscope(CaveTalk_Handle_t *const handle, const cavetalk_Gyroscope *const gyroscope);
CaveTalk_Message_t *CaveTalk_SpeakEncoders(CaveTalk_Handle_t *const handle, cavetalk_Encoder *const encoders, const size_t count);

#ifdef __cplusplus
}
#endif

#endif /* CAVETALK_H */
