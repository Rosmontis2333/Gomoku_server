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
    //该位置已经有子或对局已经结束则不执行操作
    if (board[x][y]||last_move==0) {
        refresh_clients();
        return;
    }
    //根据上一个行棋者判断当前谁改行棋
    //是该行棋的玩家就修改棋盘
    if (name==player1&&last_move==2) {
        board[x][y]=2;
        last_move=1;
        std::cout<<name<<' '<<x<<' '<<y<<std::endl;
    }else if (name==player2&&last_move==1) {
        board[x][y]=1;
        last_move=2;
        std::cout<<name<<' '<<x<<' '<<y<<std::endl;
    }else {
        //都不成立则不执行操作
        return;
    }
    //刷新客户端
    refresh_clients();
    //检查当前行棋玩家是否胜利
    if (win_check(x,y)) {
        //如果胜利就结束该局游戏
        end_game();
    }
}

void Room::end_game() {
    //判断获取胜利者名称
    winner = last_move==1?player1:player2;
    //修改战绩
    if (last_move==1) {
        playerManager.change_record(player1,0);
        playerManager.change_record(player2,1);
    }else if (last_move==2) {
        playerManager.change_record(player1,1);
        playerManager.change_record(player2,0);
    }
    //刷新客户端
    refresh_clients();
    last_move=0;
}

bool Room::win_check(int x, int y) const {
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
    //对局已结束则交换双方位置，开始下一局游戏
    if (last_move==0) {
        std::swap(player1,player2);
        winner.clear();
        last_move=2;
        board.clear();
        board.resize(17,std::vector<int>(17,0));
        refresh_clients();
        return;
    }
    //已经加入则无需操作
    if (name==player1||name==player2) {
        return;
    }
    //判断剩余空位
    if (player1.empty()) {
        player1=name;
    }else if (player2.empty()) {
        player2=name;
    }else {
        //没有空位则作为旁观者加入
        spectators.insert(name);
    }
    //刷新客户端
    refresh_clients();
    std::cout<<name<<"joined"<<roomID<<std::endl;
}

void Room::leave(const std::string &name) {
    //哪个玩家离开就清空哪个字符串
    if (player1==name) {
        player1.clear();
    }
    if (player2==name) {
        player2.clear();
    }
    //也从旁观者中移除
    spectators.erase(name);
    //刷新客户端
    refresh_clients();
}

void Room::refresh_clients() {
    std::set<unsigned long long> client_sockets;
    //获取所有玩家与旁观者的socket
    client_sockets.insert(playerManager.get_player(player1)->client_socket);
    client_sockets.insert(playerManager.get_player(player2)->client_socket);
    for (auto s:spectators) {
        client_sockets.insert(playerManager.get_player(s)->client_socket);
    }
    //构造消息
    json response;
    response["player1"]=player1.empty()?"None":player1+' '+playerManager.get_record(player1);
    response["player2"]=player2.empty()?"None":player2+' '+playerManager.get_record(player2);
    response["last_move"]=last_move;
    response["board"]=board;
    response["winner"]=winner;
    //将消息转换为字符串
    const std::string str = response.dump();
    //向所有客户端发送消息
    for (const auto socket:client_sockets) {
        send(socket, str.c_str(), str.size(), 0);
        std::cout<<"reply"<< std::endl;
    }
}
