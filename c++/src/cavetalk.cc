#include "cavetalk.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
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

const std::string_view kDelimiter = "/";

template <typename T>
static std::optional<Message> Parse(const std::vector<std::uint8_t> &data);
template <typename T>
static std::optional<std::pair<std::string, std::vector<std::uint8_t>>> Serialize(const Peer peer, const T &message);

static const std::unordered_map<std::string_view, std::optional<Message> (*)(const std::vector<std::uint8_t> &)> kParsers = {
    {Log::descriptor()->name(), Parse<Log>},
    {Arm::descriptor()->name(), Parse<Arm>},
    {Drive::descriptor()->name(), Parse<Drive>},
    {Acceleration::descriptor()->name(), Parse<Acceleration>},
    {Gyroscope::descriptor()->name(), Parse<Gyroscope>},
    {Encoders::descriptor()->name(), Parse<Encoders>},
};

std::optional<Message> Hear(const std::string_view key, const std::vector<std::uint8_t> &data)
{
    std::optional<Message> message       = std::nullopt;
    const std::size_t      name_position = key.find(kDelimiter);

    if (std::string::npos != name_position)
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

std::optional<std::pair<std::string, std::vector<std::uint8_t>>> Speak(const Peer peer, const Message &message)
{
    return std::visit([peer](const auto &serialized)
    {
        return Serialize(peer, serialized);
    }, message);
}

template <typename T>
static std::optional<Message> Parse(const std::vector<std::uint8_t> &data)
{
    std::optional<Message> parsed = std::nullopt;
    T                      message;

    if (message.ParseFromArray(data.data(), data.size()))
    {
        parsed = message;
    }

    return parsed;
}

template <typename T>
static std::optional<std::pair<std::string, std::vector<std::uint8_t>>> Serialize(const Peer peer, const T &message)
{
    std::optional<std::pair<std::string, std::vector<std::uint8_t>>> serialized = std::nullopt;
    const std::size_t                                                size       = message.ByteSizeLong();
    std::vector<std::uint8_t>                                        data(size);

    if (message.SerializeToArray(data.data(), size))
    {
        serialized = std::make_pair(GetKey<T>(peer), std::move(data));
    }

    return serialized;
}

} // namespace cavetalk
