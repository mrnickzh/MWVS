#include <iostream>
#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXNetSystem.h>
#include <Server.hpp>
#include "Protocol/ServerPacketHelper.hpp"
#include <mutex>

int main() {
    ix::initNetSystem();

    int port = 20202;
    std::string host("127.0.0.1");
    ix::WebSocketServer server(port, host);
    Server& serverInstance = Server::getInstance();
    std::mutex serverInstanceMutex;

    serverInstance.setCallback([&](ClientSession* session, std::vector<uint8_t> data) {
        std::string str(data.begin(), data.end());
        static_cast<ix::WebSocket*>(serverInstance.clients[session])->sendBinary(str);
    });

    server.setOnClientMessageCallback([&](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr & msg) {
        std::string remoteaddr = std::string(connectionState->getRemoteIp()) + " " + std::to_string(connectionState->getRemotePort());

        if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "Connected: " << remoteaddr << std::endl;
            std::lock_guard<std::mutex> guard(serverInstanceMutex);
            serverInstance.clients[new ClientSession(remoteaddr)] = (void*)&webSocket;
        }
        else if (msg->type == ix::WebSocketMessageType::Message && msg->binary)
        {
            std::vector<uint8_t> data(msg->str.begin(), msg->str.end());
            auto it = std::find_if(serverInstance.clients.begin(), serverInstance.clients.end(), [remoteaddr](std::pair<ClientSession*, void*> f)->bool{ return f.first->clientaddress == remoteaddr; });
            if (it != serverInstance.clients.end()) {
                std::lock_guard<std::mutex> guard(serverInstanceMutex);
                ServerPacketHelper::decodePacket(it->first, data);
            }
        }
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            std::cout << "Disconnected: " << remoteaddr << std::endl;
            auto it = std::find_if(serverInstance.clients.begin(), serverInstance.clients.end(), [remoteaddr](std::pair<ClientSession*, void*> f)->bool{ return f.first->clientaddress == remoteaddr; });
            if (it != serverInstance.clients.end()) {
                std::lock_guard<std::mutex> guard(serverInstanceMutex);
                serverInstance.clients.erase(it);
            }
        }
    });

    auto res = server.listen();
    if (!res.first)
    {
        return 1;
    }

    server.disablePerMessageDeflate();
    server.start();
    server.wait();

    ix::uninitNetSystem();

    return 0;
}
