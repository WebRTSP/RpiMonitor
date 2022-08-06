#include "Session.h"

#include <glib.h>

#include "RtspParser/RtspParser.h"


Session::Session(
    const Config* config,
    const std::function<std::unique_ptr<WebRTCPeer> (const std::string& uri)>& createPeer,
    const std::function<void (const rtsp::Request*)>& sendRequest,
    const std::function<void (const rtsp::Response*)>& sendResponse) noexcept :
    ServerSession(config->iceServers, createPeer,sendRequest, sendResponse),
    _config(config)
{
}

Session::Session(
    const Config* config,
    const std::function<std::unique_ptr<WebRTCPeer> (const std::string& uri)>& createPeer,
    const std::function<std::unique_ptr<WebRTCPeer> (const std::string& uri)>& createRecordPeer,
    const std::function<void (const rtsp::Request*)>& sendRequest,
    const std::function<void (const rtsp::Response*)>& sendResponse) noexcept :
    ServerSession( config->iceServers, createPeer,createRecordPeer, sendRequest, sendResponse),
    _config(config)
{
}

bool Session::recordEnabled(const std::string& uri) noexcept
{
    return true;
}

bool Session::authorize(const std::unique_ptr<rtsp::Request>& requestPtr) noexcept
{
    return true;
}
