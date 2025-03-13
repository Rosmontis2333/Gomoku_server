//
// Created by Wind_64 on 25-3-8.
//

#ifndef PLAYER_H
#define PLAYER_H

#include <fstream>
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
public:
    Player* get_player(const std::string& name) {
        if (players.contains(name)) {
            return players[name].get();
        }
        players[name] = std::make_unique<Player>();
        players[name]->name = name;
        return players[name].get();
    }

    std::string get_record(std::string name) {
        if (name.empty()) {
            return "";
        }
        read();
        std::string result;
        if (!records.contains(name)) {
            records[name] = {0,0};
            save();
        }
        result = std::to_string(records[name][0])+'-'+std::to_string(records[name][1]);
        return result;
    }
    void change_record(std::string name,int i) {
        read();
        if (records.contains(name)) {
            records[name][i]++;
        }else {
            get_record(name);
            records[name][i]++;
        }
        save();
    }
private:
    std::unordered_map<std::string, std::unique_ptr<Player>> players;
    std::map<std::string,std::vector<int>> records;
    void read() {
        std::ifstream inFile("records.json");
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string content = buffer.str();
        json j = json::parse(content);
        records.clear();
        records=j;
        inFile.close();
    }
    void save() {
        std::ofstream outFile("records.json");
        json j = records;
        std::string content = j.dump(4);
        outFile << content;
        outFile.close();
    }
};

extern PlayerManager playerManager;

#endif //PLAYER_H
