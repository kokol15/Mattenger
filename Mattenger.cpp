//
//  Mattenger.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Mattenger.hpp"

bool CONNECTION_ALIVE = false;

Mattenger::Mattenger(const char *addr){
    
    Socket::create_comm_point(addr);
    Socket::bind_socket();
    
}

void Mattenger::handshake(){
    char icmp[ICMP_HEAD] = {SYN};
    Socket::send(icmp, ICMP_HEAD);
    long length = Socket::recieve(icmp, ICMP_HEAD);
    if(length < 0)
        print_error("Failed to recieve massage");
    if(icmp[0] == SYN_ACK){
        icmp[0] = ACK;
        Socket::send(icmp, ICMP_HEAD);
        CONNECTION_ALIVE = true;
    }
}

void Mattenger::send_msg(const char *msg, size_t size){
    
    if(CONNECTION_ALIVE){
        int i;
        char *s_msg = (char*)calloc(100, sizeof(char));
        for(i = 0; msg[i] != 0; i++)
            s_msg[i] = msg[i];
        s_msg[i] = 0;
        
        Socket::send(s_msg, i);
    }
    
    else{
        handshake();
        send_msg(msg, size);
    }
}

void Mattenger::recive_msg(){
    
    while(true){
        char *msg = (char*)calloc(100, sizeof(char));
        long length = Socket::recieve(msg, 100);
        
        if(length < 0)
            print_error("Failed to recieve massage");
        else
            std::cout << "Sprava: "<< msg << std::endl;
    }
    
}

void Mattenger::listen_for_connection(){
    
    while(true){
        char *icmp = (char*)calloc(ICMP_HEAD, sizeof(char));
        if(Socket::listen(icmp, ICMP_HEAD) < 0)
            print_error("Failed to recieve massage");
        else if(icmp[0] == SYN){
            icmp[0] = SYN_ACK;
            Socket::send(icmp, ICMP_HEAD);
            if(Socket::recieve(icmp, ICMP_HEAD) < 0)
                print_error("Failed to recieve massage");
            else if(icmp[0] == ACK){
                CONNECTION_ALIVE = true;
                std::thread t2(&Mattenger::recive_msg, this);
                t2.detach();
            }
        }
    }
}

void Mattenger::start(){
    std::thread t1(&Mattenger::recive_msg, this);
    t1.detach();
}
