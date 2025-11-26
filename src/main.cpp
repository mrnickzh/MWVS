#include <iostream>
#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXNetSystem.h>
#include <Server.hpp>
#include "Protocol/ServerPacketHelper.hpp"

std::map<ClientSession, ix::WebSocket*> clients;

int main() {
    ix::initNetSystem();

    int port = 20202;
    std::string host("127.0.0.1");
    ix::WebSocketServer server(port, host);
    Server& serverInstance = Server::getInstance();

    serverInstance.setCallback([&](ClientSession session, std::vector<uint8_t> data) {
        std::string str(data.begin(), data.end());
        clients[session]->sendBinary(str);
    });

    server.setOnClientMessageCallback([&](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr & msg) {
        std::string remoteaddr = std::string(connectionState->getRemoteIp()) + " " + std::to_string(connectionState->getRemotePort());

        std::cout << "Client: " << remoteaddr << std::endl;

        if (msg->type == ix::WebSocketMessageType::Open)
        {
            clients[ClientSession(remoteaddr)] = &webSocket;
        }
        else if (msg->type == ix::WebSocketMessageType::Message && msg->binary)
        {
            std::vector<uint8_t> data(msg->str.begin(), msg->str.end());
            ServerPacketHelper::decodePacket(ClientSession(remoteaddr), data);
        }
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            clients.erase(ClientSession(remoteaddr));
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
