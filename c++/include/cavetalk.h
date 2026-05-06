#ifndef CAVETALK_H
#define CAVETALK_H

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "acceleration.pb.h"
#include "arm.pb.h"
#include "drive.pb.h"
#include "encoders.pb.h"
#include "gyroscope.pb.h"
#include "ids.pb.h"
#include "log.pb.h"

namespace cavetalk
{

using Id_t      = std::uint32_t;
using Message_t = std::pair<std::string, std::vector<std::uint8_t>>;

class Callbacks
{
    public:
        virtual ~Callbacks();
        virtual void HearLog(const std::string &log);
        virtual void HearArm(const Mode mode);
        virtual void HearDrive(const Drive &drive);
        virtual void HearAcceleration(const Acceleration &acceleration);
        virtual void HearGyroscope(const Gyroscope &gyroscope);
        virtual void HearEncoders(const Encoders &encoders);
};

class CaveTalker
{
    public:
        CaveTalker(const Id_t id, std::shared_ptr<Callbacks> callbacks);
        CaveTalker(CaveTalker &cavetalker)                  = delete;
        CaveTalker(CaveTalker &&cavetalker)                 = delete;
        CaveTalker &operator=(const CaveTalker &cavetalker) = delete;
        CaveTalker &operator=(CaveTalker &&cavetalker)      = delete;
        void Hear(const std::string &key, const std::vector<std::uint8_t> &data) const;
        Message_t SpeakLog(const std::string &log) const;
        Message_t SpeakArm(const Mode mode) const;
        Message_t SpeakDrive(const Drive &drive) const;
        Message_t SpeakAcceleration(const Acceleration &acceleration) const;
        Message_t SpeakGyroscope(const Gyroscope &gyroscope) const;
        Message_t SpeakEncoders(const Encoders &encoders) const;

    private:
        void HandleLog(const std::vector<std::uint8_t> &data) const;
        void HandleArm(const std::vector<std::uint8_t> &data) const;
        void HandleDrive(const std::vector<std::uint8_t> &data) const;
        void HandleAcceleration(const std::vector<std::uint8_t> &data) const;
        void HandleGyroscope(const std::vector<std::uint8_t> &data) const;
        void HandleEncoders(const std::vector<std::uint8_t> &data) const;

        Id_t id_;
        std::shared_ptr<Callbacks> callbacks_;
};

std::string GetKey(const Id_t peer_id, const Id key_id);

} // namespace cavetalk

#endif // CAVETALK_H
