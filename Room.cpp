//
// Created by Wind_64 on 25-3-8.
//
#include "Room.h"
#include <string>
#include <winsock2.h>
#include "Player.h"
#include "json.hpp"
using json = nlohmann::json;

RoomManager roomManager;
void Room::move(const std::string& name,int x,int y) {
    std::cout<<name<<" move"<<std::endl;
    if (board[x][y]||last_move==0) {
        refresh_clients();
        return;
    }
    if (name==player1&&last_move==2) {
        board[x][y]='2';
        last_move=1;
        std::cout<<name<<' '<<x<<' '<<y<<std::endl;
    }else if (name==player2&&last_move==1) {
        board[x][y]='1';
        last_move=2;
        std::cout<<name<<' '<<x<<' '<<y<<std::endl;
    }
    refresh_clients();
    if (checkWin(x,y)) {
        end_game();
    }
}

void Room::end_game() {
    winner = last_move==1?player1:player2;
    refresh_clients();
    last_move=0;
}

bool Room::checkWin(int x, int y) const {
    int player = board[x][y];
    if (player==0) {
        return false;
    }
    std::vector<std::pair<int, int>> directions = {
        {0, 1}, {1, 0}, {1, 1}, {1, -1}
    };
    for (const auto& dir : directions) {
        int count = 1; // 包含当前落子
        // 检查正方向
        for (int i = 1; i < 5; ++i) {
            int new_row = x + i * dir.first;
            int new_col = y + i * dir.second;
            if (new_row < 1 || new_row > 15 || new_col < 1 || new_col > 15 || board[new_row][new_col] != player)
                break;
            count++;
        }
        // 检查反方向
        for (int i = 1; i < 5; ++i) {
            int new_row = x - i * dir.first;
            int new_col = y - i * dir.second;
            if (new_row < 1 || new_row > 15 || new_col < 1 || new_col > 15 || board[new_row][new_col] != player)
                break;
            count++;
        }
        if (count >= 5) return true;
    }
    return false;
}

void Room::join(const std::string &name) {
    if (last_move==0) {
        std::swap(player1,player2);
        winner.clear();
        last_move=2;
        board.clear();
        board.resize(17,std::vector<int>(17,0));
        refresh_clients();
        return;
    }
    if (name==player1||name==player2) {
        return;
    }
    if (player1.empty()) {
        player1=name;
    }else if (player2.empty()) {
        player2=name;
    }else {
        spectators.insert(name);
    }
    refresh_clients();
    std::cout<<name<<"joined"<<roomID<<std::endl;
}

void Room::leave(const std::string &name) {
    if (player1==name) {
        player1.clear();
    }
    if (player2==name) {
        player2.clear();
    }
    spectators.erase(name);
    refresh_clients();
}
void Room::refresh_clients() {
    std::set<unsigned long long> client_sockets;
    client_sockets.insert(playerManager.get_player(player1)->client_socket);
    client_sockets.insert(playerManager.get_player(player2)->client_socket);
    for (auto s:spectators) {
        client_sockets.insert(playerManager.get_player(s)->client_socket);
    }
    json response;
    response["player1"]=player1;
    response["player2"]=player2;
    response["last_move"]=last_move;
    response["board"]=board;
    response["winner"]=winner;
    const std::string str = response.dump();
    for (const auto socket:client_sockets) {
        send(socket, str.c_str(), str.size(), 0);
        std::cout<<"reply"<< std::endl;
    }
}
