//
//  Mattenger.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Mattenger.hpp"

bool CONNECTION_ALIVE = false;
short FRAGMENT_SIZE = 2;
char *MSG[65535];

Mattenger::Mattenger(const char *addr){
    
    Socket::create_comm_point(addr);
    Socket::bind_socket();
    
}

/*void Mattenger::handshake(){
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
}*/

void Mattenger::send_msg(const char *msg, size_t size){
    
    if(CONNECTION_ALIVE){
        short i = 0, k, j;
        
        
        short num = size/FRAGMENT_SIZE;
        if(size % FRAGMENT_SIZE == 1)
            num++;
        
        char _msg_[HEAD + size + 1];
        _msg_[0] = num & 0xff;
        _msg_[1] = (num >> 8) & 0xff;
        
        while(i < num){
            k = 0; j = HEAD;
            _msg_[0] = i & 0xff;
            _msg_[1] = (i >> 8) & 0xff;
            while(msg[k] != 0) _msg_[j++] = msg[k++];
            _msg_[j++] = 0;
            Socket::send(_msg_, j);
            std::this_thread::sleep_for (std::chrono::milliseconds(500));
            i++;
        }
        
        char end[ICMP_HEAD] = {DATA_END};
        Socket::send(end, ICMP_HEAD);
        
    }
    
    else{
        char icmp_msg[ICMP_HEAD] = {SYN};
        Socket::send(icmp_msg, ICMP_HEAD);
        std::this_thread::sleep_for (std::chrono::milliseconds(500));
        send_msg(msg, size);
    }
}

void Mattenger::recive_msg(){
    
    char icmp_msg[ICMP_HEAD];
    char *msg = (char*)calloc(100, sizeof(char));
    short tot_num, seq_num;
    int i, j;
    std::string recreate_msg;
    
    do{
        
        long length = Socket::recieve(msg, 100);
        
        if(length < 0)
            print_error("Failed to recieve massage");
        else
            switch(msg[0]){
                case SYN:
                    icmp_msg[0] = {SYN_ACK};
                    Socket::send(icmp_msg, ICMP_HEAD);
                    break;
                    
                case SYN_ACK:
                    icmp_msg[0] = {ACK};
                    CONNECTION_ALIVE = true;
                    Socket::send(icmp_msg, ICMP_HEAD);
                    break;
                    
                case ACK:
                    CONNECTION_ALIVE = true;
                    break;
                    
                case DATA_END:
                    i = 0;
                    j = 0;
                    while(MSG[i] != 0){
                        while(MSG[i][j] != 0)
                            recreate_msg += MSG[i][j++];
                        i++;
                    }
                    break;
                    
                default:
                    if(CONNECTION_ALIVE){
                        memcpy(&tot_num, msg, sizeof(char));
                        memcpy(&seq_num, (msg + sizeof(char)), sizeof(char));
                        
                        MSG[seq_num] = (char*)calloc(FRAGMENT_SIZE + 1, sizeof(char));
                        i = sizeof(short);
                        j = 0;
                        while(msg[i] != 0){
                            MSG[seq_num][j++] = msg[i++];
                        }
                    }
                    break;
            }
    }while(true);
    
}

/*void Mattenger::listen_for_connection(){
    
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
}*/

void Mattenger::start(){
    std::thread t1(&Mattenger::recive_msg, this);
    t1.detach();
}
