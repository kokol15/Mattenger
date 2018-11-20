//
//  Socket.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Socket.hpp"

Socket::Socket(){
    
    int s1 = socket(AF_INET, SOCK_DGRAM, 0);
    int s2 = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(s1 < 0){
        std::cerr << "Failed to create sending socket: ERROR " << errno << std::endl;
        return;
    }
    else
        this -> s_soc = s1;
    
    if(s2 < 0){
        std::cerr << "Failed to create recieving socket: ERROR " << errno << std::endl;
        return;
    }
    else
        this -> r_soc = s2;
    
}

void Socket::create_comm_point(const char *address, int port){
    
    struct sockaddr_in s_addr;
    struct sockaddr_in r_addr;
    memset(&s_addr, 0, sizeof(s_addr));
    memset(&r_addr, 0, sizeof(r_addr));
    char arr[4];
    int a, b, c, d;
    
    sscanf(address, "%d.%d.%d.%d", &a, &b, &c, &d);
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d;
    
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(50050);
    if(arr[0] == 127 && arr[1] == 0 && arr[2] == 0 && arr[3] == 1)
        s_addr.sin_addr.s_addr = INADDR_ANY;
    else
        s_addr.sin_addr.s_addr = *(unsigned int*)arr;
    
    r_addr.sin_family = AF_INET;
    r_addr.sin_port = htons(50040);
    r_addr.sin_addr.s_addr = INADDR_ANY;
    
    this -> send_address = s_addr;
    this -> rcv_address = r_addr;
    
}

void Socket::bind_socket(){
    
    if (bind(this -> s_soc, (struct sockaddr *)&this -> send_address, sizeof(this -> send_address)) < 0 ) {
        std::cerr << "Failed to bind sending socket: ERROR " << errno << std::endl;
        return;
    }
    
    if (bind(this -> r_soc, (struct sockaddr *)&this -> rcv_address, sizeof(this -> rcv_address)) < 0 ) {
        std::cerr << "Failed to bind recieving socket: ERROR " << errno << std::endl;
        return;
    }
    
}

ssize_t Socket::listen(char *output, size_t size){
    
    int r = this -> r_soc;
    return recvfrom(r, output, size, 0, (struct sockaddr*)&this -> rcv_address, &this -> rcv_addr_size);
    
}

ssize_t Socket::recieve(char *output, size_t size){
    
    int r = this -> r_soc;
    return recv(r, output, size, 0);
    
}

void Socket::send(char *msg, size_t size){
    
    int s = this -> s_soc;
    socklen_t addr_size = sizeof(this -> send_address);
    sendto(s, msg, size, 0, (struct sockaddr*)&this -> send_address, addr_size);

}

//********* GETTERS AND SETTERS ***********//
struct sockaddr_in Socket::get_send_address(){
    return this -> send_address;
}

struct sockaddr_in Socket::get_rcv_address(){
    return this -> rcv_address;
}
