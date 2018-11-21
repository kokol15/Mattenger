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
        print_error("Failed to create sending socket");
        return;
    }
    else
        this -> s_soc = s1;
    
    if(s2 < 0){
        print_error("Failed to create recieving socket");
        return;
    }
    else
        this -> r_soc = s2;
    
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
    s_addr.sin_port = htons(50050);
    if(strcmp(address, "127.0.0.1") == 0)
        s_addr.sin_addr.s_addr = INADDR_ANY;
    else
        s_addr.sin_addr.s_addr = inet_addr(address);
    
    r_addr.sin_family = AF_INET;
    r_addr.sin_port = htons(50050);
    r_addr.sin_addr.s_addr = INADDR_ANY;
    
    this -> send_address = s_addr;
    this -> rcv_address = r_addr;
    this -> rcv_addr_size = sizeof(r_addr);
    
}

void Socket::bind_socket(){
    
    if (bind(this -> r_soc, (struct sockaddr *)&this -> rcv_address, this -> rcv_addr_size) < 0 ) {
        print_error("Failed to bind recieving socket");
        return;
    }
    
}

ssize_t Socket::listen(char *output, size_t size){
    
    int r = this -> r_soc;
    long retval = recvfrom(r, output, size, 0, (struct sockaddr*)&this -> send_address, &this -> send_addr_size);
    return retval;
    
}

ssize_t Socket::recieve(char *output, size_t size){
    
    int r = this -> s_soc;
    return recv(r, output, size, 0);
    
}

void Socket::send(char *msg, size_t size){
    std::this_thread::sleep_for (std::chrono::seconds(1));
    
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
