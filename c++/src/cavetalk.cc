#include "cavetalk.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "acceleration.pb.h"
#include "arm.pb.h"
#include "drive.pb.h"
#include "encoders.pb.h"
#include "gyroscope.pb.h"
#include "log.pb.h"

#define CAVETALK_UNUSED(arg) (void)(arg)

namespace cavetalk
{

const std::string_view kDelimiter = "/";

template <typename T>
static ParsedMessage Parse(const std::vector<std::uint8_t> &data);
template <typename T>
static SerializedMessage Serialize(const Peer peer, const T &message);

static const std::unordered_map<std::string_view, ParsedMessage (*)(const std::vector<std::uint8_t> &)> kParsers = {
    {Log::descriptor()->name(), Parse<Log>},
    {Arm::descriptor()->name(), Parse<Arm>},
    {Drive::descriptor()->name(), Parse<Drive>},
    {Acceleration::descriptor()->name(), Parse<Acceleration>},
    {Gyroscope::descriptor()->name(), Parse<Gyroscope>},
    {Encoders::descriptor()->name(), Parse<Encoders>},
};

ParsedMessage Hear(const std::string_view key, const std::vector<std::uint8_t> &data)
{
    ParsedMessage     message       = std::nullopt;
    const std::size_t name_position = key.find(kDelimiter);

    if (std::string_view::npos != name_position)
    {
        const std::string_view name   = key.substr(name_position + kDelimiter.length());
        const auto             parser = kParsers.find(name);

        if (kParsers.end() != parser)
        {
            message = parser->second(data);
        }
    }

    return message;
}

ParsedMessage Hear(const KeyDataPair &key_data)
{
    return Hear(key_data.key, key_data.data);
}

SerializedMessage Speak(const Peer peer, const Message &message)
{
    return std::visit([peer](const auto &arg)
    {
        return Serialize(peer, arg);
    }, message);
}

template <typename T>
static ParsedMessage Parse(const std::vector<std::uint8_t> &data)
{
    ParsedMessage parsed = std::nullopt;
    T             message;

    if (message.ParseFromArray(data.data(), data.size()))
    {
        parsed = message;
    }

    return parsed;
}

template <typename T>
static SerializedMessage Serialize(const Peer peer, const T &message)
{
    SerializedMessage         serialized = std::nullopt;
    const std::size_t         size       = message.ByteSizeLong();
    std::vector<std::uint8_t> data(size);

    if (message.SerializeToArray(data.data(), size))
    {
        serialized = {.key = GetKey<T>(peer), .data = std::move(data)};
    }

    return serialized;
}

} // namespace cavetalk
