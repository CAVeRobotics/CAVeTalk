#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include "cavetalk.h"

static const cavetalk::Peer kPeer = 1234U;

MATCHER_P(EqualsProto, expected, "")
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

TEST(CaveTalkCpp, GetKey)
{
    ASSERT_EQ("1234/Drive", cavetalk::GetKey<cavetalk::Drive>(1234U));
    ASSERT_EQ("123456/Acceleration", cavetalk::GetKey<cavetalk::Acceleration>(123456U));
    ASSERT_EQ("1234/Log", cavetalk::GetKey<cavetalk::Log>(1234U));
    ASSERT_EQ("123456/SetMode", cavetalk::GetKey<cavetalk::SetMode>(123456U));
    ASSERT_EQ("123456/GetMode", cavetalk::GetKey<cavetalk::GetMode>(123456U));
}

TEST(CaveTalkCpp, SpeakAndHearLog)
{
    const std::string log_0 = "foobar";
    const std::string log_1 = "baz";

    cavetalk::Log log_message;
    log_message.set_log(log_0);
    cavetalk::SerializedMessage serialized_message = cavetalk::Speak(kPeer, log_message);
    ASSERT_NE(std::nullopt, serialized_message);
    cavetalk::KeyDataPair key_data = serialized_message.value();
    cavetalk::ParsedMessage parsed_message = cavetalk::Hear(key_data);
    ASSERT_NE(std::nullopt, parsed_message);
    ASSERT_EQ(log_0, std::get<cavetalk::Log>(parsed_message.value()).log());

    log_message.set_log(log_1);
    serialized_message = cavetalk::Speak(kPeer, log_message);
    ASSERT_NE(std::nullopt, serialized_message);
    key_data = serialized_message.value();
    parsed_message = cavetalk::Hear(key_data);
    ASSERT_NE(std::nullopt, parsed_message);
    ASSERT_EQ(log_1, std::get<cavetalk::Log>(parsed_message.value()).log());
}

TEST(CaveTalkCpp, SpeakAndHearMode)
{
    cavetalk::SetMode mode_message;
    mode_message.set_mode(cavetalk::Mode::MODE_DISARMED);
    cavetalk::SerializedMessage serialized_message = cavetalk::Speak(kPeer, mode_message);
    ASSERT_NE(std::nullopt, serialized_message);
    cavetalk::KeyDataPair key_data = serialized_message.value();
    cavetalk::ParsedMessage parsed_message = cavetalk::Hear(key_data);
    ASSERT_NE(std::nullopt, parsed_message);
    ASSERT_EQ(cavetalk::Mode::MODE_DISARMED, std::get<cavetalk::SetMode>(parsed_message.value()).mode());

    mode_message.set_mode(cavetalk::Mode::MODE_ARMED_AUTO);
    serialized_message = cavetalk::Speak(kPeer, mode_message);
    ASSERT_NE(std::nullopt, serialized_message);
    key_data = serialized_message.value();
    parsed_message = cavetalk::Hear(key_data);
    ASSERT_NE(std::nullopt, parsed_message);
    ASSERT_EQ(cavetalk::Mode::MODE_ARMED_AUTO, std::get<cavetalk::SetMode>(parsed_message.value()).mode());
}

TEST(CaveTalkCpp, SpeakAndHearAcceleration)
{
    cavetalk::Acceleration acceleration_message;
    acceleration_message.set_x_meters_per_second_squared(0.0f);
    acceleration_message.set_y_meters_per_second_squared(0.0f);
    acceleration_message.set_z_meters_per_second_squared(0.0f);
    cavetalk::SerializedMessage serialized_message = cavetalk::Speak(kPeer, acceleration_message);
    ASSERT_NE(std::nullopt, serialized_message);
    cavetalk::KeyDataPair key_data = serialized_message.value();
    cavetalk::ParsedMessage parsed_message = cavetalk::Hear(key_data);
    ASSERT_NE(std::nullopt, parsed_message);
    ASSERT_EQ(0.0f, std::get<cavetalk::Acceleration>(parsed_message.value()).x_meters_per_second_squared());
    ASSERT_EQ(0.0f, std::get<cavetalk::Acceleration>(parsed_message.value()).y_meters_per_second_squared());
    ASSERT_EQ(0.0f, std::get<cavetalk::Acceleration>(parsed_message.value()).z_meters_per_second_squared());

    acceleration_message.set_x_meters_per_second_squared(1.2f);
    acceleration_message.set_y_meters_per_second_squared(3.4f);
    acceleration_message.set_z_meters_per_second_squared(5.6f);
    serialized_message = cavetalk::Speak(kPeer, acceleration_message);
    ASSERT_NE(std::nullopt, serialized_message);
    key_data = serialized_message.value();
    parsed_message = cavetalk::Hear(key_data);
    ASSERT_NE(std::nullopt, parsed_message);
    ASSERT_EQ(1.2f, std::get<cavetalk::Acceleration>(parsed_message.value()).x_meters_per_second_squared());
    ASSERT_EQ(3.4f, std::get<cavetalk::Acceleration>(parsed_message.value()).y_meters_per_second_squared());
    ASSERT_EQ(5.6f, std::get<cavetalk::Acceleration>(parsed_message.value()).z_meters_per_second_squared());
}

TEST(CaveTalkCpp, SpeakAndHearEncoders)
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

    cavetalk::SerializedMessage serialized_message = cavetalk::Speak(kPeer, encoders_0);
    ASSERT_NE(std::nullopt, serialized_message);
    cavetalk::KeyDataPair key_data = serialized_message.value();
    cavetalk::ParsedMessage parsed_message = cavetalk::Hear(key_data);
    ASSERT_NE(std::nullopt, parsed_message);
    ASSERT_THAT(std::get<cavetalk::Encoders>(parsed_message.value()), EqualsProto(encoders_0));

    cavetalk::Encoders encoders_1;
    encoder = encoders_1.add_encoders();
    encoder->set_pulses(-130);
    encoder->set_rate_radians_per_second(1.4f);
    encoder = encoders_1.add_encoders();
    encoder->set_pulses(260);
    encoder->set_rate_radians_per_second(-2.7f);

    serialized_message = cavetalk::Speak(kPeer, encoders_1);
    ASSERT_NE(std::nullopt, serialized_message);
    key_data = serialized_message.value();
    parsed_message = cavetalk::Hear(key_data);
    ASSERT_NE(std::nullopt, parsed_message);
    ASSERT_THAT(std::get<cavetalk::Encoders>(parsed_message.value()), EqualsProto(encoders_1));
}
