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
        //当前不在一个房间里则不执行操作
        if (room==nullptr||RoomID==0){
            return;
        }
        //解析客户端传递的消息
        const int x=j["data"]["position"]["x"].get<int>();
        const int y=j["data"]["position"]["y"].get<int>();
        //调用房间对象的行棋方法
        room->move(name,x,y);
    };
    void join(json j) {
        //如果当前已经在一个房间里则先退出房间
        if (room!=nullptr&&RoomID!=j["data"]["roomID"])room->leave(name);
        //解析客户端发送的消息
        RoomID = j["data"]["roomID"].get<int>();
        room=roomManager.get_room(RoomID);
        //调用房间对象的加入方法
        room->join(name);
    }
};

class PlayerManager {
public:
    Player* get_player(const std::string& name) {
        //已经存在则直接返回指针
        if (players.contains(name)) {
            return players[name].get();
        }
        //不存在则创建再返回指针
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
        //没有记录就先创建再保存
        if (!records.contains(name)) {
            records[name] = {0,0};
            save();
        }
        //拼接战绩字符串
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
    //私有变量和方法
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
