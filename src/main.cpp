#include <iostream>
#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXNetSystem.h>
#include <Server.hpp>
#include "Protocol/ServerPacketHelper.hpp"
#include <mutex>
#include "../libs/json.hpp"
#include "Protocol/Packets/EntityActionServer.hpp"
#include <fstream>
#include <WorldSaving/RegionRegistory.hpp>

nlohmann::json config;

void save() {
    std::ofstream savedJson("config.json");
    if (savedJson) {
        savedJson << config.dump(4);
    }
}

void load() {
    std::ifstream file("config.json");

    if (!file) {
        config["host"] = "0.0.0.0";
        config["port"] = 20202;

        save();
    } else file >> config;
}

int main(int argc, char* argv[]) {
    load();
    std::string host = config["host"];
    int port = config["port"];

    ix::initNetSystem();
    ix::WebSocketServer server(port, host);
    Server& serverInstance = Server::getInstance();
    std::mutex serverInstanceMutex;

    if (std::filesystem::exists("world.mww")) {
        RegionRegistory::getInstance().importAll();
    }

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
                std::string tempuuid = std::move(it->first->uuid);
                serverInstance.clients.erase(it);
                serverInstance.entities.erase(ServerEntity(tempuuid, Vec3<float>(0.0f, 0.0f, 0.0f), Vec3<float>(0.0f, 0.0f, 0.0f), Vec3<float>(0.0f, 0.0f, 0.0f)));
                for (auto clt : serverInstance.clients) {
                    if (clt.first) {
                        EntityActionServer replicationpacket;
                        replicationpacket.uuid = tempuuid;
                        replicationpacket.action = 1;
                        Server::getInstance().sendPacket(clt.first, &replicationpacket);
                    }
                }
            }
        }
    });

    std::cout << "Starting server on " << host << ":" << port << "..." << std::endl;

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
