//
// Created by Wind_64 on 25-3-8.
//

#include "Network.h"
#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h>
#include <thread>
#include "Player.h"
#include "json.hpp"
using json = nlohmann::json;

#define PORT 23333
static void cleanup_winsock() {
    WSACleanup();
}

static bool init_winsock(){
  WSADATA wsaData;
  int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "Winsock 初始化失败: " << result << std::endl;
        return false;
    }
    return true;
}


void Network::handle_client(SOCKET client_socket) {
    char buffer[1024] = {};
    int bytes_read;

    Player* player=nullptr;
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        std::string str(buffer);
        json j = json::parse(str);
        std::string action = j["action"].get<std::string>();
        std::string name = j["data"]["playerName"].get<std::string>();
        if (action == "login") {
            player = playerManager.get_player(name);
            player->client_socket = client_socket;
            std::cout<<name<<" login"<<std::endl;
        }else if(action == "move"&&player!=nullptr){
            player->move(j);
        }else if(action == "join"&&player!=nullptr){
            player->join(j);
        }
        // 清空 buffer 以接收下一条消息
        memset(buffer, 0, sizeof(buffer));
    }

    if (bytes_read == 0) {
        std::cout << "客户端断开连接" << std::endl;
    } else {
        std::cerr << "读取客户端消息时出错" << std::endl;
    }

    // 关闭与客户端的连接
    closesocket(client_socket);
}


void Network::initialize() {
    if (!init_winsock()) {
        return;
    }

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);

    // 创建套接字
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "套接字创建失败: " << WSAGetLastError() << std::endl;
        cleanup_winsock();
        return;
    }

    // 配置服务端地址结构
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 监听所有网络接口
    server_addr.sin_port = htons(PORT);        // 端口号

    // 绑定套接字到地址
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "绑定失败: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        cleanup_winsock();
        return;
    }

    // 监听端口
    if (listen(server_socket, 100) == SOCKET_ERROR) {
        std::cerr << "监听失败: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        cleanup_winsock();
        return;
    }

    std::cout << "listening on " << PORT << std::endl;

    while (true) {
        // 接受客户端连接
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "connection failed " << WSAGetLastError() << std::endl;
            continue;  // 继续监听其他连接
        }

        std::cout << "connected" << std::endl;

        // 创建新线程处理客户端消息
        std::thread client_thread(Network::handle_client, client_socket);
		client_thread.detach();
    }

    // 关闭服务端套接字
    closesocket(server_socket);

    // 清理 Winsock
    cleanup_winsock();

    return;
}