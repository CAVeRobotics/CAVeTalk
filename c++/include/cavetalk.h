#ifndef CAVETALK_H
#define CAVETALK_H

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "acceleration.pb.h"
#include "drive.pb.h"
#include "encoders.pb.h"
#include "gyroscope.pb.h"
#include "log.pb.h"
#include "mode.pb.h"

namespace cavetalk
{

struct KeyDataPair;

using Peer              = std::uint32_t;
using Message           = std::variant<Acceleration, Drive, Encoders, Gyroscope, Log, SetMode, GetMode>;
using ParsedMessage     = std::optional<Message>;
using SerializedMessage = std::optional<KeyDataPair>;

extern const std::string_view kDelimiter;

struct KeyDataPair
{
    std::string key;
    std::vector<std::uint8_t> data;
};

ParsedMessage Hear(const std::string_view key, const std::vector<std::uint8_t> &data);
ParsedMessage Hear(const KeyDataPair &key_data);
SerializedMessage Speak(const Peer peer, const Message &message);

template <typename T>
std::string GetKey(const Peer peer)
{
    std::string key = std::to_string(peer);

    key += kDelimiter;
    key += T::descriptor()->name();

    return key;
}

} // namespace cavetalk

#endif // CAVETALK_H
