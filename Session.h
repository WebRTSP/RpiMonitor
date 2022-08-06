#pragma once

#include "Signalling/ServerSession.h"

#include "Config.h"


class Session : public ServerSession
{
public:
    Session(
        const Config*,
        const std::function<std::unique_ptr<WebRTCPeer> (const std::string& uri)>& createPeer,
        const std::function<void (const rtsp::Request*)>& sendRequest,
        const std::function<void (const rtsp::Response*)>& sendResponse) noexcept;
    Session(
        const Config*,
        const std::function<std::unique_ptr<WebRTCPeer> (const std::string& uri)>& createPeer,
        const std::function<std::unique_ptr<WebRTCPeer> (const std::string& uri)>& createRecordPeer,
        const std::function<void (const rtsp::Request*)>& sendRequest,
        const std::function<void (const rtsp::Response*)>& sendResponse) noexcept;

protected:
    bool listEnabled() noexcept override { return true; }
    bool recordEnabled(const std::string& uri) noexcept override;
    bool authorize(const std::unique_ptr<rtsp::Request>& requestPtr) noexcept;

private:
    const Config *const _config;
};
