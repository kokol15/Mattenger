//
//  Mattenger.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Mattenger.hpp"
#define MAX_SIZE 65535

bool CONNECTION_ALIVE = false;
short FRAGMENT_SIZE = 2;
char *MSG[MAX_SIZE];
char *O_MSG[MAX_SIZE];

void print_msg(std::string msg){
    std::cout << msg << std::endl;
}

Mattenger::Mattenger(const char *addr){
    
    Socket::create_comm_point(addr);
    Socket::bind_socket();
    
}

void Mattenger::send_msg(const char *msg, size_t size){
    
    if(CONNECTION_ALIVE){
        short i = 0, k = 0, j, _i;
        
        
        short num = size/FRAGMENT_SIZE;
        if(size % FRAGMENT_SIZE == 1)
            num++;
        
        char _msg_[HEAD + FRAGMENT_SIZE];
        memcpy(_msg_, &FRAGMENT_SIZE, sizeof(short));
        
        while(i < num){
            _i = 0;
            j = HEAD;
            memcpy((_msg_ + sizeof(short)), &i, sizeof(short));
            while( _i++ < FRAGMENT_SIZE && msg[k] != 0) _msg_[j++] = msg[k++];
            _msg_[j++] = 0;
            printf("%s|", (_msg_ + HEAD));
            Socket::send(_msg_, j);
            std::this_thread::sleep_for (std::chrono::milliseconds(100));
            i++;
        }
        printf("\n");
        
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
                    while(MSG[i] != 0)
                        recreate_msg += MSG[i];
                    print_msg(recreate_msg);
                    recreate_msg.clear();
                    for(i = 0; i < MAX_SIZE; i++){
                        free(MSG[i]);
                        MSG[i] = NULL;
                    }
                    break;
                    
                default:
                    if(CONNECTION_ALIVE){
                        memcpy(&tot_num, msg, sizeof(short));
                        memcpy(&seq_num, (msg + sizeof(short)), sizeof(short));
                        
                        MSG[seq_num] = (char*)calloc(tot_num + 1, sizeof(char));
                        i = HEAD;
                        j = 0;
                        while(j < tot_num){
                            MSG[seq_num][j++] = msg[i++];
                        }
                        MSG[seq_num][j] = 0;
                    }
                    break;
            }
    }while(true);
    
}

void Mattenger::start(){
    std::thread t1(&Mattenger::recive_msg, this);
    t1.detach();
}
