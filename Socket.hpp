//
//  Socket.hpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#ifndef Socket_hpp
#define Socket_hpp

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>

class Socket{
    
private:
    int s_soc;
    int r_soc;
    struct sockaddr_in send_address;
    struct sockaddr_in rcv_address;
    socklen_t send_addr_size;
    socklen_t rcv_addr_size;
    
    
public:
    Socket();
    void print_error(std::string err);
    void create_comm_point(const char *, int);
    void bind_socket();
    void send(char*, size_t);
    struct sockaddr_in get_send_address();
    struct sockaddr_in get_rcv_address();
    ssize_t recieve(char *, size_t);
    ssize_t listen(char *, size_t);
    
};

#endif /* Socket_hpp */
