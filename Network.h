//
// Created by Wind_64 on 25-3-8.
//

#ifndef NETWORK_H
#define NETWORK_H
#include <winsock2.h>

class Network {
  public:
    static void handle_client(SOCKET client_socket);
    static void initialize();
};

#endif //NETWORK_H
