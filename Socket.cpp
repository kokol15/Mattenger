//
//  Socket.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Socket.hpp"

Socket::Socket(){
    
    int soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(soc < 0){
        print_error("Failed to create sending socket");
        return;
    }
    
    this -> soc = soc;
    
}

void Socket::print_error(std::string err){
    
    std::cerr << err + ": ERROR " << errno  << " (" << strerror(errno) << ")" << std::endl;
    
}

void Socket::create_comm_point(const char *address, int port){
    
    struct sockaddr_in s_addr;
    struct sockaddr_in r_addr;
    memset(&s_addr, 0, sizeof(s_addr));
    memset(&r_addr, 0, sizeof(r_addr));
    
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    if(strcmp(address, "127.0.0.1") == 0)
        s_addr.sin_addr.s_addr = INADDR_ANY;
    else
        s_addr.sin_addr.s_addr = inet_addr(address);
    
    r_addr.sin_family = AF_INET;
    r_addr.sin_port = htons(port);
    r_addr.sin_addr.s_addr = INADDR_ANY;
    
    this -> send_address = s_addr;
    this -> rcv_address = r_addr;
    this -> rcv_addr_size = sizeof(r_addr);
    
}

void Socket::bind_socket(){
    
    if (bind(this -> soc, (struct sockaddr *)&this -> rcv_address, this -> rcv_addr_size) < 0 ) {
        print_error("Failed to bind recieving socket");
        return;
    }
    
}

ssize_t Socket::recieve(char *output, size_t size){
    
    return recvfrom(this -> soc, output, size, 0, (struct sockaddr*)&this -> send_address, &this -> send_addr_size);
    
}

void Socket::send(const char *msg, size_t size){
//    std::this_thread::sleep_for (std::chrono::milliseconds(100));
    
    int s = this -> soc;
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
