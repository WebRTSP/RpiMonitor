#include "Monitor.h"

#include <CxxPtr/GlibPtr.h>
#include <CxxPtr/libwebsocketsPtr.h>

#include "RtStreaming/GstRtStreaming/LibGst.h"

#include "WebRTSP/Client/WsClient.h"

#include "Log.h"
#include "Session.h"


enum {
    RECONNECT_TIMEOUT = 10,
};


static std::unique_ptr<WebRTCPeer>
CreatePeer(
    const Config* config,
    const std::string& uri)
{
    return nullptr;
}

static std::unique_ptr<WebRTCPeer>
CreateRecordPeer(
    const Config* config,
    const std::string& uri)
{
    return nullptr;
}

static std::unique_ptr<rtsp::Session> CreateSession(
    const Config* config,
    const std::function<void (const rtsp::Request*) noexcept>& sendRequest,
    const std::function<void (const rtsp::Response*) noexcept>& sendResponse) noexcept
{
    std::unique_ptr<Session> session =
        std::make_unique<Session>(
            config,
            std::bind(CreatePeer, config, std::placeholders::_1),
            std::bind(CreateRecordPeer, config, std::placeholders::_1),
            sendRequest, sendResponse);

    return session;
}

static void ClientDisconnected(client::WsClient& client) noexcept
{
    GSourcePtr timeoutSourcePtr(g_timeout_source_new_seconds(RECONNECT_TIMEOUT));
    GSource* timeoutSource = timeoutSourcePtr.get();
    g_source_set_callback(timeoutSource,
        [] (gpointer userData) -> gboolean {
            static_cast<client::WsClient*>(userData)->connect();
            return false;
        }, &client, nullptr);
    g_source_attach(timeoutSource, g_main_context_get_thread_default());
}

int MonitorMain(const Config& config)
{
    LibGst libGst;

    GMainContextPtr contextPtr(g_main_context_new());
    GMainContext* context = contextPtr.get();
    g_main_context_push_thread_default(context);

    GMainLoopPtr loopPtr(g_main_loop_new(context, FALSE));
    GMainLoop* loop = loopPtr.get();

    client::WsClient client(
        config,
        loop,
        std::bind(
            CreateSession,
            &config,
            std::placeholders::_1,
            std::placeholders::_2),
        ClientDisconnected);

    if(client.init()) {
        client.connect();
        g_main_loop_run(loop);
    }

    return 0;
}
