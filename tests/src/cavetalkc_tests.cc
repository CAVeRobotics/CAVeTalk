#include <cstdint>

#include <gtest/gtest.h>

#include "cavetalk.h"

#define CAVETALK_TEST_BUFFER_SIZE 256U

class CaveTalk : public testing::Test
{
protected:
    static void HearLog(const char *const log)
    {
        printf(log);
    }

    void SetUp() override
    {
        CaveTalk_Callbacks_t listener_callbacks = kCaveTalk_CallbacksNull;
        listener_callbacks.hear_log = HearLog;

        CaveTalk_Initialize(&speaker_handle_, &kCaveTalk_CallbacksNull, speaker_id_, speaker_buffer_, sizeof(speaker_buffer_));
        CaveTalk_Initialize(&listener_handle_, &listener_callbacks, listener_id_, listener_buffer_, sizeof(listener_buffer_));
    }

    CaveTalk_Handle_t speaker_handle_;
    CaveTalk_Handle_t listener_handle_;
    CaveTalk_Id_t speaker_id_ = 1234U;
    CaveTalk_Id_t listener_id_ = 5678U;
    std::uint8_t speaker_buffer_[CAVETALK_TEST_BUFFER_SIZE];
    std::uint8_t listener_buffer_[CAVETALK_TEST_BUFFER_SIZE];
};

TEST_F(CaveTalk, SpeakAndHearLog)
{
    char log[] = "foobar";

    CaveTalk_Message_t *message = CaveTalk_SpeakLog(&speaker_handle_, log);
    CaveTalk_Hear(&listener_handle_, *message);
}
