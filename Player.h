//
// Created by Wind_64 on 25-3-8.
//

#ifndef PLAYER_H
#define PLAYER_H
#include <iostream>
#include <memory>
#include <unordered_map>
#include "Room.h"
#include "string"
#include "json.hpp"
using json = nlohmann::json;
class Player {
    public:
    std::string name;
    unsigned long long client_socket=0;
    bool IsInARoom = false;
    int RoomID=0;
    Room *room=nullptr;
    void move(json j) const {
        if (room==nullptr||RoomID==0){
            return;
        }
        const int x=j["data"]["position"]["x"].get<int>();
        const int y=j["data"]["position"]["y"].get<int>();
        room->move(name,x,y);
    };
    void join(json j) {
        if (room!=nullptr&&RoomID!=j["data"]["roomID"])room->leave(name);
        RoomID = j["data"]["roomID"].get<int>();
        room=roomManager.get_room(RoomID);
        room->join(name);
    }
};
class PlayerManager {
    std::unordered_map<std::string, std::unique_ptr<Player>> players;
    public:
    Player* get_player(const std::string& name) {
        if (players.contains(name)) {
            return players[name].get();
        }
        players[name] = std::make_unique<Player>();
        players[name]->name = name;
        return players[name].get();
    }
};

extern PlayerManager playerManager;

#endif //PLAYER_H
