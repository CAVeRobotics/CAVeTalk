#include "cavetalk.h"

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "acceleration.pb.h"
#include "arm.pb.h"
#include "drive.pb.h"
#include "encoders.pb.h"
#include "gyroscope.pb.h"
#include "ids.pb.h"
#include "log.pb.h"

#define CAVETALK_UNUSED(arg) (void)(arg)

namespace cavetalk
{

static const std::string kDelimiter = "/";
// *INDENT-OFF*
    constexpr std::array<std::string_view, ID_MAX> kTopicKeys = []
    {
        std::array<std::string_view, ID_MAX> topic_keys{};

        topic_keys[static_cast<int>(ID_NONE)] = "none";
        topic_keys[static_cast<int>(ID_LOG)] = "log";
        topic_keys[static_cast<int>(ID_ARM)] = "arm";
        topic_keys[static_cast<int>(ID_DRIVE)] = "drive";
        topic_keys[static_cast<int>(ID_ACCELERATION)] = "acceleration";
        topic_keys[static_cast<int>(ID_GYROSCOPE)] = "gyroscope";
        topic_keys[static_cast<int>(ID_ENCODERS)] = "encoders";

        return topic_keys;
    }();
// *INDENT-ON*

static Id GetId(const std::string &key);

Callbacks::~Callbacks() = default;

void Callbacks::HearLog(const std::string &log)
{
    CAVETALK_UNUSED(log);
}

void Callbacks::HearArm(const Mode mode)
{
    CAVETALK_UNUSED(mode);
}

void Callbacks::HearDrive(const Drive &drive)
{
    CAVETALK_UNUSED(drive);
}

void Callbacks::HearAcceleration(const Acceleration &acceleration)
{
    CAVETALK_UNUSED(acceleration);
}

void Callbacks::HearGyroscope(const Gyroscope &gyroscope)
{
    CAVETALK_UNUSED(gyroscope);
}

void Callbacks::HearEncoders(const Encoders &encoders)
{
    CAVETALK_UNUSED(encoders);
}

CaveTalker::CaveTalker(const Id_t id, std::shared_ptr<Callbacks> callbacks) : id_(id), callbacks_(callbacks)
{
}

void CaveTalker::Hear(const Message_t &message) const
{
    switch (GetId(message.first))
    {
    case ID_LOG:
        HandleLog(message.second);
        break;
    case ID_ARM:
        HandleArm(message.second);
        break;
    case ID_DRIVE:
        HandleDrive(message.second);
        break;
    case ID_ACCELERATION:
        HandleAcceleration(message.second);
        break;
    case ID_GYROSCOPE:
        HandleGyroscope(message.second);
        break;
    case ID_ENCODERS:
        HandleEncoders(message.second);
        break;
    case ID_NONE:
    default:
        break;
    }
}

Message_t CaveTalker::SpeakLog(const std::string &log) const
{
    Log log_message;
    log_message.set_log(log);

    std::size_t               size = log_message.ByteSizeLong();
    std::vector<std::uint8_t> data(size);

    log_message.SerializeToArray(data.data(), size);

    return Message_t{GetKey(id_, ID_LOG), data};
}

Message_t CaveTalker::SpeakArm(const Mode mode) const
{
    Arm arm_message;
    arm_message.set_mode(mode);

    std::size_t               size = arm_message.ByteSizeLong();
    std::vector<std::uint8_t> data(size);

    arm_message.SerializeToArray(data.data(), size);

    return Message_t{GetKey(id_, ID_ARM), data};
}

Message_t CaveTalker::SpeakDrive(const Drive &drive) const
{
    std::size_t               size = drive.ByteSizeLong();
    std::vector<std::uint8_t> data(size);

    drive.SerializeToArray(data.data(), size);

    return Message_t{GetKey(id_, ID_DRIVE), data};
}

Message_t CaveTalker::SpeakAcceleration(const Acceleration &acceleration) const
{
    std::size_t               size = acceleration.ByteSizeLong();
    std::vector<std::uint8_t> data(size);

    acceleration.SerializeToArray(data.data(), size);

    return Message_t{GetKey(id_, ID_ACCELERATION), data};
}

Message_t CaveTalker::SpeakGyroscope(const Gyroscope &gyroscope) const
{
    std::size_t               size = gyroscope.ByteSizeLong();
    std::vector<std::uint8_t> data(size);

    gyroscope.SerializeToArray(data.data(), size);

    return Message_t{GetKey(id_, ID_GYROSCOPE), data};
}

Message_t CaveTalker::SpeakEncoders(const Encoders &encoders) const
{
    std::size_t               size = encoders.ByteSizeLong();
    std::vector<std::uint8_t> data(size);

    encoders.SerializeToArray(data.data(), size);

    return Message_t{GetKey(id_, ID_ENCODERS), data};
}

void CaveTalker::HandleLog(const std::vector<std::uint8_t> &data) const
{
    Log log_message;

    if (log_message.ParseFromArray(data.data(), data.size()))
    {
        callbacks_->HearLog(log_message.log());
    }
}

void CaveTalker::HandleArm(const std::vector<std::uint8_t> &data) const
{
    Arm arm_message;

    if (arm_message.ParseFromArray(data.data(), data.size()))
    {
        callbacks_->HearArm(arm_message.mode());
    }
}

void CaveTalker::HandleDrive(const std::vector<std::uint8_t> &data) const
{
    Drive drive_message;

    if (drive_message.ParseFromArray(data.data(), data.size()))
    {
        callbacks_->HearDrive(drive_message);
    }
}

void CaveTalker::HandleAcceleration(const std::vector<std::uint8_t> &data) const
{
    Acceleration acceleration_message;

    if (acceleration_message.ParseFromArray(data.data(), data.size()))
    {
        callbacks_->HearAcceleration(acceleration_message);
    }
}

void CaveTalker::HandleGyroscope(const std::vector<std::uint8_t> &data) const
{
    Gyroscope gyroscope_message;

    if (gyroscope_message.ParseFromArray(data.data(), data.size()))
    {
        callbacks_->HearGyroscope(gyroscope_message);
    }
}

void CaveTalker::HandleEncoders(const std::vector<std::uint8_t> &data) const
{
    Encoders encoders_message;

    if (encoders_message.ParseFromArray(data.data(), data.size()))
    {
        callbacks_->HearEncoders(encoders_message);
    }
}

std::string GetKey(const Id_t peer_id, const Id key_id)
{
    std::string key;

    if (key_id < ID_MAX)
    {
        key += std::to_string(peer_id);
        key += kDelimiter;
        key += kTopicKeys[key_id];
    }

    return key;
}

static Id GetId(const std::string &key)
{
    Id id = ID_NONE;

    std::size_t position = key.find(kDelimiter);

    if (std::string::npos != position)
    {
        const std::string topic = key.substr(position + kDelimiter.length());

        for (std::size_t i = 0U; i < kTopicKeys.size(); i++)
        {
            if (kTopicKeys[i] == topic)
            {
                id = static_cast<Id>(i);
            }
        }
    }

    return id;
}

} // namespace cavetalk
