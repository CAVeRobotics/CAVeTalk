#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cavetalk.h"

#define CAVETALK_TEST_BUFFER_SIZE 256U

class Callbacks
{
public:
    virtual void HearLog(const char *const log) const = 0;
    virtual void HearArm(const cavetalk_Mode mode) const = 0;
    virtual void HearDrive(const cavetalk_Drive *const drive) const = 0;
    virtual void HearAcceleration(const cavetalk_Acceleration *const acceleration) const = 0;
    virtual void HearGyroscope(const cavetalk_Gyroscope *const gyroscope) const = 0;
    virtual void HearEncoders(const cavetalk_Encoder *const encoders, const std::size_t count) const = 0;
};

class MockCallbacks : public Callbacks
{
public:
    MOCK_METHOD(void, HearLog, (const char *const log), (const, override));
    MOCK_METHOD(void, HearArm, (const cavetalk_Mode mode), (const, override));
    MOCK_METHOD(void, HearDrive, (const cavetalk_Drive *const drive), (const, override));
    MOCK_METHOD(void, HearAcceleration, (const cavetalk_Acceleration *const acceleration), (const, override));
    MOCK_METHOD(void, HearGyroscope, (const cavetalk_Gyroscope *const gyroscope), (const, override));
    MOCK_METHOD(void, HearEncoders, (const cavetalk_Encoder *const encoders, const std::size_t count), (const, override));
};

class CaveTalk : public testing::Test
{
protected:
    static void HearLog(const char *const log)
    {
        mock_callbacks_->HearLog(log);
    }

    static void HearArm(const cavetalk_Mode mode)
    {
        mock_callbacks_->HearArm(mode);
    }

    static void HearDrive(const cavetalk_Drive *const drive)
    {
        mock_callbacks_->HearDrive(drive);
    }

    static void HearAcceleration(const cavetalk_Acceleration *const acceleration)
    {
        mock_callbacks_->HearAcceleration(acceleration);
    }

    static void HearGyroscope(const cavetalk_Gyroscope *const gyroscope)
    {
        mock_callbacks_->HearGyroscope(gyroscope);
    }

    static void HearEncoders(const cavetalk_Encoder *const encoders, const std::size_t count)
    {
        mock_callbacks_->HearEncoders(encoders, count);
    }

    void SetUp() override
    {
        mock_callbacks_ = new MockCallbacks;

        CaveTalk_Callbacks_t listener_callbacks = kCaveTalk_CallbacksNull;
        listener_callbacks.hear_log = HearLog;
        listener_callbacks.hear_encoders = HearEncoders;

        CaveTalk_Initialize(&speaker_handle_, &kCaveTalk_CallbacksNull, speaker_id_, speaker_buffer_, sizeof(speaker_buffer_));
        CaveTalk_Initialize(&listener_handle_, &listener_callbacks, listener_id_, listener_buffer_, sizeof(listener_buffer_));
    }

    void TearDown() override
    {
        delete mock_callbacks_;
    }

    static MockCallbacks *mock_callbacks_;

    CaveTalk_Handle_t speaker_handle_;
    CaveTalk_Handle_t listener_handle_;
    CaveTalk_Id_t speaker_id_ = 1234U;
    CaveTalk_Id_t listener_id_ = 5678U;
    std::uint8_t speaker_buffer_[CAVETALK_TEST_BUFFER_SIZE];
    std::uint8_t listener_buffer_[CAVETALK_TEST_BUFFER_SIZE];
};

MockCallbacks *CaveTalk::mock_callbacks_ = nullptr;

TEST_F(CaveTalk, GetKey)
{
    ASSERT_STREQ("1234/drive", CaveTalk_GetKey(&speaker_handle_, 1234U, cavetalk_Id_ID_DRIVE));
    ASSERT_STREQ("123456/acceleration", CaveTalk_GetKey(&speaker_handle_, 123456U, cavetalk_Id_ID_ACCELERATION));
    ASSERT_STREQ("1234/log", CaveTalk_GetKey(&speaker_handle_, 1234U, cavetalk_Id_ID_LOG));
    ASSERT_STREQ("123456/arm", CaveTalk_GetKey(&speaker_handle_, 123456U, cavetalk_Id_ID_ARM));
}

TEST_F(CaveTalk, SpeakAndHearLog)
{
    char log_0[] = "foobar";
    char log_1[] = "baz";

    CaveTalk_Message_t *message = CaveTalk_SpeakLog(&speaker_handle_, log_0);
    ASSERT_NE(nullptr, message);
    EXPECT_CALL(*mock_callbacks_, HearLog(testing::StrEq(log_0))).Times(1);
    CaveTalk_Hear(&listener_handle_, *message);

    message = CaveTalk_SpeakLog(&speaker_handle_, log_1);
    ASSERT_NE(nullptr, message);
    EXPECT_CALL(*mock_callbacks_, HearLog(testing::StrEq(log_1))).Times(1);
    CaveTalk_Hear(&listener_handle_, *message);
}

TEST_F(CaveTalk, SpeakAndHearEncoders)
{
    cavetalk_Encoder encoders_0[] = {
        {.pulses = 100U, .rate_radians_per_second = 1.5f},
        {.pulses = 250U, .rate_radians_per_second = -2.6f},
        {.pulses = -370U, .rate_radians_per_second = 1.7f},
    };
    size_t encoders_0_count = sizeof(encoders_0) / sizeof(encoders_0[0U]);
    cavetalk_Encoder encoders_1[] = {
        {.pulses = -130U, .rate_radians_per_second = 1.4f},
        {.pulses = 260U, .rate_radians_per_second = -2.7f},
    };
    size_t encoders_1_count = sizeof(encoders_1) / sizeof(encoders_1[0U]);

    CaveTalk_Message_t *message = CaveTalk_SpeakEncoders(&speaker_handle_, encoders_0, encoders_0_count);
    EXPECT_CALL(*mock_callbacks_, HearEncoders(
                                      testing::Truly([&encoders_0, &encoders_0_count](const cavetalk_Encoder *const encoders)
                                                     {
                                                        for (std::size_t i = 0U; i < encoders_0_count; i++)
                                                        {
                                                            if ((encoders_0[i].pulses != encoders[i].pulses) || (encoders_0[i].rate_radians_per_second != encoders[i].rate_radians_per_second))
                                                            {
                                                                return false;
                                                            }
                                                        }
                                                        return true; }),
                                      encoders_0_count))
        .Times(1);
    CaveTalk_Hear(&listener_handle_, *message);

    message = CaveTalk_SpeakEncoders(&speaker_handle_, encoders_1, sizeof(encoders_1) / sizeof(encoders_1[0U]));
    EXPECT_CALL(*mock_callbacks_, HearEncoders(
                                      testing::Truly([&encoders_1, &encoders_1_count](const cavetalk_Encoder *const encoders)
                                                     {
                                                          for (std::size_t i = 0U; i < encoders_1_count; i++)
                                                          {
                                                              if ((encoders_1[i].pulses != encoders[i].pulses) || (encoders_1[i].rate_radians_per_second != encoders[i].rate_radians_per_second))
                                                              {
                                                                  return false;
                                                              }
                                                          }
                                                          return true; }),
                                      encoders_1_count))
        .Times(1);
    CaveTalk_Hear(&listener_handle_, *message);
}
