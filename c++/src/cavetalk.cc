#include "cavetalk.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
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

static const std::string                         kDelimiter = "/";
static const std::unordered_map<std::string, Id> kTopicKeys = {
    {"none", ID_NONE},
    {"log", ID_LOG},
    {"arm", ID_ARM},
    {"drive", ID_DRIVE},
    {"acceleration", ID_ACCELERATION},
    {"gyroscope", ID_GYROSCOPE},
    {"encoders", ID_ENCODERS},
};

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

void CaveTalker::Hear(const std::string &key, const std::vector<std::uint8_t> &data) const
{
    switch (GetId(key))
    {
    case ID_LOG:
        HandleLog(data);
        break;
    case ID_ARM:
        HandleArm(data);
        break;
    case ID_DRIVE:
        HandleDrive(data);
        break;
    case ID_ACCELERATION:
        HandleAcceleration(data);
        break;
    case ID_GYROSCOPE:
        HandleGyroscope(data);
        break;
    case ID_ENCODERS:
        HandleEncoders(data);
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
    return std::to_string(peer_id) + kDelimiter + std::to_string(key_id);
}

static Id GetId(const std::string &key)
{
    Id id = ID_NONE;

    std::size_t position = key.find(kDelimiter);

    if (std::string::npos != position)
    {
        std::unordered_map<std::string, Id>::const_iterator i = kTopicKeys.find(key.substr(position + kDelimiter.length()));

        if (kTopicKeys.end() != i)
        {
            id = i->second;
        }
    }

    return id;
}

} // namespace cavetalk
