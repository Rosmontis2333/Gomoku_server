//
// Created by Wind_64 on 25-3-8.
//

#ifndef ROOM_H
#define ROOM_H


#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
#include <string>

class Room {
    public:
    Room() {
        board.resize(17,std::vector<int>(17,0));
    }
    int roomID=0;
    void move(const std::string& name,int x,int y);
    bool win_check(int x,int y) const;
    void end_game();
    void refresh_clients();
    void join(const std::string &name);
    void leave(const std::string &name);
    std::string player1;
    std::string player2;
    std::string winner;
    int last_move=2;
    //1表示玩家1,2表示玩家2,0表示对局结束
    std::set<std::string> spectators;
    std::vector<std::vector<int>> board;
};

class RoomManager{
    std::unordered_map<int, std::unique_ptr<Room>> rooms;
    public:
    Room* get_room(const int& roomID) {
        //已经存在则直接返回指针
        if (rooms.contains(roomID)) {
            return rooms[roomID].get();
        }
        //不存在则创建再返回指针
        rooms[roomID] = std::make_unique<Room>();
        rooms[roomID]->roomID = roomID;
        return rooms[roomID].get();
    }

};
extern RoomManager roomManager;

#endif //ROOM_H
