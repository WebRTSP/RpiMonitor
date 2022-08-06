#pragma once

#include <deque>

#include <spdlog/common.h>

#include "Client/Config.h"


struct Config: public client::Config
{
    spdlog::level::level_enum logLevel = spdlog::level::info;
    spdlog::level::level_enum lwsLogLevel = spdlog::level::warn;

    typedef std::deque<std::string> IceServers;
    IceServers iceServers;

    std::string recordToken;
};
