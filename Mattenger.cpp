//
//  Mattenger.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Mattenger.hpp"
#define MAX_SIZE 65535
#define A 54059 /* a prime */
#define B 76963 /* another prime */
#define C 86969 /* yet another prime */
#define FIRST 37 /* also prime */

bool CONNECTION_ALIVE = false;
bool ALTER_CRC = true;
short FRAGMENT_SIZE = 2;
short FRAG_TOTAL_NUM;
char *MSG[MAX_SIZE];
char *_MSG_[MAX_SIZE];

void print_msg(std::string msg){
    std::cout << msg << std::endl;
}

ssize_t get_size(char *msg){
    ssize_t i = 0;
    while(msg[i] != 0) i++;
    return i;
}

unsigned short computeCRC(const char* s)
{
    unsigned short h = FIRST;
    while (*s) {
        h = (h * A) ^ (s[0] * B);
        s++;
    }
    return (h % MAX_SIZE);
}

Mattenger::Mattenger(const char *addr){
    
    Socket::create_comm_point(addr);
    Socket::bind_socket();
    
}

void Mattenger::send_msg(const char *msg, size_t size){
    
    if(CONNECTION_ALIVE){
        short i = 0, k = 0, j, _i;
        int l;
        
        for(l = 0; l < MAX_SIZE; l++){
            free(_MSG_[l]);
            _MSG_[l] = NULL;
        }
        
        short num = size/FRAGMENT_SIZE;
        if(size % FRAGMENT_SIZE == 1)
            num++;
        
        char _msg_[HEAD + FRAGMENT_SIZE];
        memcpy(_msg_, &FRAGMENT_SIZE, sizeof(short));
        memcpy((_msg_ + 2*sizeof(short)), &num, sizeof(short));
        
        while(i < num){
            _i = 0;
            j = HEAD;
            
            memcpy((_msg_ + sizeof(short)), &i, sizeof(short));
            while( _i++ < FRAGMENT_SIZE && msg[k] != 0) _msg_[j++] = msg[k++];
            
            _msg_[j++] = 0;
            printf("%s|", (_msg_ + HEAD));
            
            unsigned short crc = computeCRC((_msg_ + HEAD));
            memcpy((_msg_ + 3*sizeof(short)), &crc, sizeof(short));
            _MSG_[i] = (char*)calloc(j, sizeof(char));
            memcpy(_MSG_[i], _msg_, j*sizeof(char));
            
            if(ALTER_CRC){
                _msg_[HEAD] = '.';
                ALTER_CRC = false;
            }
            
            
            Socket::send(_msg_, j);
            //std::this_thread::sleep_for (std::chrono::milliseconds(100));
            i++;
        }
        printf("\n");
        
        char end[ICMP_HEAD] = {DATA_END};
        Socket::send(end, ICMP_HEAD);
        
    }
    
    else{
        char icmp_msg[ICMP_HEAD] = {SYN};
        Socket::send(icmp_msg, ICMP_HEAD);
        //std::this_thread::sleep_for (std::chrono::milliseconds(500));
        send_msg(msg, size);
    }
}

void Mattenger::recive_msg(){
    
    char *msg = (char*)calloc(100, sizeof(char));
    int i = 0, j = 0;
    std::string recreate_msg;
    std::string resend;
    std::string _resend_;
    char icmp_msg[ICMP_HEAD];
    short seq_num, size_num;
    
    do{
        
        long length = Socket::recieve(msg, 100);
        unsigned short crc;
        unsigned short _crc_;
        
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
                    for(j = 0; j < FRAG_TOTAL_NUM; j++)
                        if(MSG[j] == NULL)
                            resend.push_back(j);
                    
                    if(resend.size() > 0){
                        _resend_.push_back(RESEND);
                        _resend_ += resend;
                        _resend_.push_back(-1);
                        
                        Socket::send(_resend_.c_str(), _resend_.size());
                        _resend_.clear();
                        resend.clear();
                        break;
                    }
                    
                    i = 0;
                    while(MSG[i] != NULL)
                        recreate_msg += MSG[i++];
                    
                    print_msg(recreate_msg);
                    recreate_msg.clear();
                    
                    for(i = 0; i < MAX_SIZE; i++){
                        free(MSG[i]);
                        MSG[i] = NULL;
                    }
                    
                    break;
                    
                case RESEND:
                    i = 1;
                    while(msg[i] >= 0){
                        Socket::send(_MSG_[msg[i]], get_size(_MSG_[msg[i]]));
                        i++;
                        //std::this_thread::sleep_for (std::chrono::milliseconds(100));
                    }
                        
                    break;
                    
                default:
                    if(CONNECTION_ALIVE){
                        
                        memcpy(&size_num, msg, sizeof(short));
                        memcpy(&seq_num, (msg + sizeof(short)), sizeof(short));
                        memcpy(&FRAG_TOTAL_NUM, (msg + 2*sizeof(short)), sizeof(short));
                        memcpy(&crc, (msg + 3*sizeof(short)), sizeof(short));
                        
                        _crc_ = computeCRC((msg + HEAD));
                        if(_crc_ != crc){
                            
                            std::cout << "Message has been altered" << std::endl;
                            
                            _resend_.push_back(RESEND);
                            _resend_.push_back(seq_num);
                            _resend_.push_back(-1);
                            
                            Socket::send(_resend_.c_str(), _resend_.size());
                            _resend_.clear();
                            
                            break;
                        }
                        
                        MSG[seq_num] = (char*)calloc(size_num + 1, sizeof(char));
                        memcpy(MSG[seq_num], (msg + HEAD), size_num*sizeof(char));
                        
                        MSG[seq_num][size_num] = 0;
                    }
                    break;
            }
    }while(true);
    
}

void Mattenger::start(){
    std::thread t1(&Mattenger::recive_msg, this);
    t1.detach();
}
