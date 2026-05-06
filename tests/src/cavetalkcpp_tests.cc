#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include "cavetalk.h"

class MockCallbacks : public cavetalk::Callbacks
{
public:
    MOCK_METHOD(void, HearLog, (const std::string &log), (override));
    MOCK_METHOD(void, HearArm, (const cavetalk::Mode mode), (override));
    MOCK_METHOD(void, HearDrive, (const cavetalk::Drive &drive), (override));
    MOCK_METHOD(void, HearAcceleration, (const cavetalk::Acceleration &acceleration), (override));
    MOCK_METHOD(void, HearGyroscope, (const cavetalk::Gyroscope &gyroscope), (override));
    MOCK_METHOD(void, HearEncoders, (const cavetalk::Encoders &encoders), (override));
};

class CaveTalkCpp : public testing::Test
{
public:
    CaveTalkCpp() : callbacks_(std::make_shared<MockCallbacks>()), cavetalker_(1234U, callbacks_) {}

protected:
    std::shared_ptr<MockCallbacks> callbacks_;
    cavetalk::CaveTalker cavetalker_;
};

MATCHER_P(ProtoEq, expected, "")
{
    google::protobuf::util::MessageDifferencer differencer;
    std::string difference;
    bool matched = true;

    differencer.ReportDifferencesToString(&difference);

    if (!differencer.Compare(expected, arg))
    {
        *result_listener << "protobufs differ:\n"
                         << difference;
        matched = false;
    }

    return matched;
}

TEST_F(CaveTalkCpp, GetKey)
{
    ASSERT_EQ("1234/drive", cavetalk::GetKey(1234U, cavetalk::ID_DRIVE));
    ASSERT_EQ("123456/acceleration", cavetalk::GetKey(123456U, cavetalk::ID_ACCELERATION));
    ASSERT_EQ("1234/log", cavetalk::GetKey(1234U, cavetalk::ID_LOG));
    ASSERT_EQ("123456/arm", cavetalk::GetKey(123456U, cavetalk::ID_ARM));
}

TEST_F(CaveTalkCpp, SpeakAndHearLog)
{
    const std::string log_0 = "foobar";
    const std::string log_1 = "baz";

    cavetalk::Message_t message = cavetalker_.SpeakLog(log_0);
    EXPECT_CALL(*callbacks_, HearLog(testing::StrEq(log_0))).Times(1);
    cavetalker_.Hear(message);

    message = cavetalker_.SpeakLog(log_1);
    EXPECT_CALL(*callbacks_, HearLog(testing::StrEq(log_1))).Times(1);
    cavetalker_.Hear(message);
}

TEST_F(CaveTalkCpp, SpeakAndHearEncoders)
{
    cavetalk::Encoders encoders_0;
    cavetalk::Encoder *encoder = encoders_0.add_encoders();
    encoder->set_pulses(100);
    encoder->set_rate_radians_per_second(1.5f);
    encoder = encoders_0.add_encoders();
    encoder->set_pulses(250);
    encoder->set_rate_radians_per_second(-2.6f);
    encoder = encoders_0.add_encoders();
    encoder->set_pulses(-370);
    encoder->set_rate_radians_per_second(1.7f);

    cavetalk::Message_t message = cavetalker_.SpeakEncoders(encoders_0);
    EXPECT_CALL(*callbacks_, HearEncoders(ProtoEq(encoders_0))).Times(1);
    cavetalker_.Hear(message);

    cavetalk::Encoders encoders_1;
    encoder = encoders_1.add_encoders();
    encoder->set_pulses(-130);
    encoder->set_rate_radians_per_second(1.4f);
    encoder = encoders_1.add_encoders();
    encoder->set_pulses(260);
    encoder->set_rate_radians_per_second(-2.7f);

    message = cavetalker_.SpeakEncoders(encoders_1);
    EXPECT_CALL(*callbacks_, HearEncoders(ProtoEq(encoders_1))).Times(1);
    cavetalker_.Hear(message);
}
