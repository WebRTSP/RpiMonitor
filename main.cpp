#include <deque>

#include <glib.h>

#include <libwebsockets.h>

#include <CxxPtr/CPtr.h>
#include "CxxPtr/libconfigDestroy.h"

#include "Helpers/ConfigHelpers.h"
#include "Helpers/LwsLog.h"

#include "Http/Log.h"
#include "Http/Config.h"

#include "RtStreaming/GstRtStreaming/Log.h"
#include "RtStreaming/GstRtStreaming/LibGst.h"

#include "Signalling/Log.h"

#include "Log.h"
#include "Monitor.h"


static const auto Log = MonitorLog;


static bool LoadConfig(Config* config)
{
    const std::deque<std::string> configDirs = ::ConfigDirs();
    if(configDirs.empty())
        return false;

    Config loadedConfig = *config;

    for(const std::string& configDir: configDirs) {
        const std::string configFile = configDir + "/restreamer.conf";
        if(!g_file_test(configFile.c_str(), G_FILE_TEST_IS_REGULAR)) {
            Log()->info("Config \"{}\" not found", configFile);
            continue;
        }

        config_t config;
        config_init(&config);
        ConfigDestroy ConfigDestroy(&config);

        Log()->info("Loading config \"{}\"", configFile);
        if(!config_read_file(&config, configFile.c_str())) {
            Log()->error("Fail load config. {}. {}:{}",
                config_error_text(&config),
                configFile,
                config_error_line(&config));
            return false;
        }

        config_setting_t* stunServerConfig = config_lookup(&config, "stun");
        if(stunServerConfig && CONFIG_TRUE == config_setting_is_group(stunServerConfig)) {
            const char* stunServer = nullptr;
            if(CONFIG_TRUE == config_setting_lookup_string(stunServerConfig, "server", &stunServer)) {
                if(0 == g_ascii_strncasecmp(stunServer, "stun://", 7)) {
                    loadedConfig.iceServers.emplace_back(stunServer);
                } else {
                    Log()->error("STUN server URL should start with \"stun://\"");
                }
            }
        }

        const char* stunServer = nullptr;
        if(CONFIG_TRUE == config_lookup_string(&config, "stun-server", &stunServer)) {
            if(0 == g_ascii_strncasecmp(stunServer, "stun://", 7)) {
                loadedConfig.iceServers.emplace_back(stunServer);
            } else {
                Log()->error("STUN server URL should start with \"stun://\"");
            }
        }

        const char* turnServer = nullptr;
        if(CONFIG_TRUE == config_lookup_string(&config, "turn-server", &turnServer)) {
           if(0 == g_ascii_strncasecmp(turnServer, "turn://", 7)) {
                loadedConfig.iceServers.emplace_back(turnServer);
            } else {
                Log()->error("TURN server URL should start with \"turn://\"");
           }
        }

        config_setting_t* debugConfig = config_lookup(&config, "debug");
        if(debugConfig && CONFIG_TRUE == config_setting_is_group(debugConfig)) {
            int logLevel = 0;
            if(CONFIG_TRUE == config_setting_lookup_int(debugConfig, "log-level", &logLevel)) {
                if(logLevel > 0) {
                    loadedConfig.logLevel =
                        static_cast<spdlog::level::level_enum>(
                            spdlog::level::critical - std::min<int>(logLevel, spdlog::level::critical));
                }
            }
            int lwsLogLevel = 0;
            if(CONFIG_TRUE == config_setting_lookup_int(debugConfig, "lws-log-level", &lwsLogLevel)) {
                if(lwsLogLevel > 0) {
                    loadedConfig.lwsLogLevel =
                        static_cast<spdlog::level::level_enum>(
                            spdlog::level::critical - std::min<int>(lwsLogLevel, spdlog::level::critical));
                }
            }
        }

        config_setting_t* streamersConfig = config_lookup(&config, "streamers");
        if(streamersConfig && CONFIG_TRUE == config_setting_is_list(streamersConfig)) {
            const int streamersCount = config_setting_length(streamersConfig);
            for(int streamerIdx = 0; streamerIdx < streamersCount; ++streamerIdx) {
                config_setting_t* streamerConfig =
                    config_setting_get_elem(streamersConfig, streamerIdx);
                if(!streamerConfig || CONFIG_FALSE == config_setting_is_group(streamerConfig)) {
                    Log()->warn("Wrong streamer config format. Streamer skipped.");
                    break;
                }
                const char* name;
                if(CONFIG_FALSE == config_setting_lookup_string(streamerConfig, "name", &name)) {
                    Log()->warn("Missing streamer name. Streamer skipped.");
                    break;
                }

                const char* recordToken = "";
                config_setting_lookup_string(streamerConfig, "record-token", &recordToken);

                const char* type = nullptr;
                config_setting_lookup_string(streamerConfig, "type", &type);

                const char* description = nullptr;
                config_setting_lookup_string(streamerConfig, "description", &description);

                const char* forceH264ProfileLevelId = nullptr;
                config_setting_lookup_string(streamerConfig, "force-h264-profile-level-id", &forceH264ProfileLevelId);


                CharPtr escapedNamePtr(
                    g_uri_escape_string(name, nullptr, false));
                if(!escapedNamePtr)
                    break; // insufficient memory?

            }
        }
    }

    bool success = true;

    if(success) {
        *config = loadedConfig;
    }

    return success;
}

int main(int argc, char *argv[])
{
    Config config {};
    if(!LoadConfig(&config))
        return -1;

    InitLwsLogger(config.lwsLogLevel);
    InitHttpServerLogger(config.logLevel);
    InitWsServerLogger(config.logLevel);
    InitServerSessionLogger(config.logLevel);
    InitGstRtStreamingLogger(config.logLevel);
    InitMonitorLogger(config.logLevel);

    LibGst libGst;

    return MonitorMain(config);
}
