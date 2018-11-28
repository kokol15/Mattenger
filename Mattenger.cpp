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
#define POLY 0x8408

bool CONNECTION_ALIVE = false;
bool SENDING_FINNISHED = true;
bool KEEPALIVE = true;
bool ALTER_CRC = false;
bool FILE_SEND = false;
bool FILENAME_SEND = false;
unsigned short FRAGMENT_SIZE = 50;
unsigned short FRAG_TOTAL_NUM;
char **MSG;
char **_MSG_;
std::string FILENAME;

void create_file(std::string f_data){
    
    std::ofstream outfile;
    outfile.open(FILENAME, std::ios::out | std::ios::trunc );
    outfile << f_data;
    outfile.close();
    
}

unsigned short computeCRC(const char* s, unsigned short n){
    
    unsigned short i = 0;
    unsigned short h = FIRST;
    while (i < n) {
        h = (h * A) ^ (s[i] * B);
        i++;
    }
    return (h % MAX_SIZE);
}

void recive_data(char *msg){
    
    unsigned short size_num = 0, seq_num = 0, crc = 0, _crc_ = 0;
    
    if(CONNECTION_ALIVE){
        
        memcpy(&size_num, (msg + FRAGMENT_SIZE_INFO), sizeof(unsigned short));
        memcpy(&seq_num, (msg + FRAGMENT_SEQNUM_INFO), sizeof(unsigned short));
        memcpy(&FRAG_TOTAL_NUM, (msg + FRAGMENT_NUM_INFO), sizeof(unsigned short));
        memcpy(&crc, (msg + FRAGMENT_CRC_INFO), sizeof(unsigned short));
        
        _crc_ = computeCRC((msg + HEAD), size_num);
        if(_crc_ != crc){
            std::cout << "Message has been altered" << std::endl;
            return;
        }
        
        MSG[seq_num] = (char*)calloc(size_num + 1, sizeof(char));
        memcpy(MSG[seq_num], (msg + HEAD), size_num*sizeof(char));
        
        MSG[seq_num][size_num] = 0;
    }
    
}

Mattenger::Mattenger(const char *addr, int port, int frag_size){
    
    
    Socket::create_comm_point(addr, port);
    Socket::bind_socket();
    FRAGMENT_SIZE = frag_size;
    
}

void Mattenger::keep_alive(){
    
    std::this_thread::sleep_for (std::chrono::seconds(120));
    
    while(this -> KEEPALIVE){
        
        this -> KEEPALIVE = false;
        char keep_alive[ICMP_HEAD] = {KEEP_ALIVE};
        Socket::send(keep_alive, ICMP_HEAD);
        std::this_thread::sleep_for (std::chrono::seconds(5));
        
    }
    
    std::cout << "Disconected" << std::endl;
    exit(1);
    
}

std::string Mattenger::check_message(){
    
    char resend[MAX_SIZE] = {0};
    
    unsigned short i = 0, j = 0, size = 0;
    std::string recreate_msg;
    
    resend[0] = RESEND;
    for(j = 0; j < FRAG_TOTAL_NUM; j++){
        if(MSG[j] == NULL){
            size = i*sizeof(unsigned short) + sizeof(char);
            unsigned short tmp = j;
            tmp++;
            memcpy((resend + size), &tmp, sizeof(unsigned short));
            i++;
        }
        else
            recreate_msg += MSG[j];
    }
    
    if(i > 0){
        //memcpy((resend + i*sizeof(unsigned short) + sizeof(char)), 0, sizeof(unsigned short));
        Socket::send(resend, i*sizeof(unsigned short) + sizeof(char));
        return "";
    }
    
    return recreate_msg;
    
}

void Mattenger::finnish_sending(){
    
    char icmp_msg[ICMP_HEAD];
    
    icmp_msg[0] = DONE_SENDING;
    MSG = (char**)calloc(MAX_SIZE, sizeof(char*));
    Socket::send(icmp_msg, ICMP_HEAD);
    
}

void Mattenger::send_msg(const char *msg, size_t size, char flag, bool crc_altered){
    
    if(crc_altered)
        ALTER_CRC = true;
    
    if(CONNECTION_ALIVE){
        
        if(SENDING_FINNISHED){
            
            SENDING_FINNISHED = false;
            FILENAME_SEND = false;
            FILE_SEND = false;
            
            if(flag == FILE_NAME)
                FILENAME_SEND = true;
            else if(flag == FILE_DATA)
                FILE_SEND = true;
            
            unsigned short i = 0, j, _i;
            unsigned long k = 0;
            unsigned short num = size/FRAGMENT_SIZE;
            if(size % FRAGMENT_SIZE > 0)
                num++;
            
            char _msg_[HEAD + FRAGMENT_SIZE];
            memcpy((_msg_ + FRAGMENT_NUM_INFO), &num, sizeof(unsigned short));
            
            while(i < num){
                _i = 0;
                j = HEAD;
                
                memcpy((_msg_ + FRAGMENT_SEQNUM_INFO), &i, sizeof(unsigned short));
                while( _i++ < FRAGMENT_SIZE && k < size)
                    _msg_[j++] = msg[k++];
                
                short tmp = _i - 1;
                memcpy((_msg_ + FRAGMENT_SIZE_INFO), &tmp, sizeof(unsigned short));
                unsigned short crc = computeCRC((_msg_ + HEAD), j - HEAD);
                memcpy((_msg_ + FRAGMENT_CRC_INFO), &crc, sizeof(unsigned short));
                
                _MSG_[i] = (char*)calloc(1, (j + 1)*sizeof(char));
                memcpy(_MSG_[i], _msg_, j*sizeof(char));
                _MSG_[i][j] = 0;
                
                if(i == (num - 1) && crc_altered)
                    ALTER_CRC = true;
                if(ALTER_CRC){
                    int pos = rand() % _i;
                    _msg_[pos + HEAD] = '@';
                    ALTER_CRC = false;
                }
                
                Socket::send(_msg_, j);
                std::this_thread::sleep_for (std::chrono::milliseconds(50));
                i++;
            }
            printf("\n");
            
            char end[ICMP_HEAD] = {flag};
            Socket::send(end, ICMP_HEAD);
        }
        
    }
    
    else{
        char icmp_msg[ICMP_HEAD] = {SYN};
        Socket::send(icmp_msg, ICMP_HEAD);
        std::this_thread::sleep_for (std::chrono::milliseconds(50));
        send_msg(msg, size, flag, crc_altered);
    }
}

void Mattenger::recive_msg(){
    
    char *msg = (char*)calloc(MAX_SIZE, sizeof(char));
    unsigned short i = 0, j = 0, n = 0;
    std::string recreate_msg;
    std::string _resend_;
    char icmp_msg[ICMP_HEAD];
    
    do{
        
        long length = Socket::recieve(msg, MAX_SIZE);
        
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
                    _MSG_ = (char**)calloc(MAX_SIZE, sizeof(char*));
                    MSG = (char**)calloc(MAX_SIZE, sizeof(char*));
                    CONNECTION_ALIVE = true;
                    Socket::send(icmp_msg, ICMP_HEAD);
                    break;
                    
                case ACK:
                    _MSG_ = (char**)calloc(MAX_SIZE, sizeof(char*));
                    MSG = (char**)calloc(MAX_SIZE, sizeof(char*));
                    CONNECTION_ALIVE = true;
                    break;
                    
                case MESSAGE:
                    recreate_msg = Mattenger::check_message();
                    
                    if(recreate_msg.empty())
                        break;
                    std::cout << recreate_msg << std::endl;
                    recreate_msg.clear();
                    
                    Mattenger::finnish_sending();
                    break;
                    
                case FILE_NAME:
                    recreate_msg = Mattenger::check_message();
                    
                    if(recreate_msg.empty())
                        break;
                    
                    FILENAME = recreate_msg;
                    recreate_msg.clear();
                    
                    Mattenger::finnish_sending();
                    break;
                    
                case FILE_DATA:
                    recreate_msg = Mattenger::check_message();
                    
                    if(recreate_msg.empty())
                        break;
                    
                    create_file(recreate_msg);
                    recreate_msg.clear();
                    
                    Mattenger::finnish_sending();
                    printf("Súbor %s bol doručený\n", FILENAME.c_str());
                    FILENAME.clear();
                    break;
                    
                case RESEND:
                    i = 1;
                    j = 0;
                    memcpy(&j, (msg + sizeof(char)), sizeof(unsigned short));
                    while(j != 0){
                        j--;
                        memcpy(&n, (_MSG_[j] + FRAGMENT_SIZE_INFO), sizeof(unsigned short));
                        n += HEAD;
                        Socket::send(_MSG_[j], n);
                        std::this_thread::sleep_for (std::chrono::milliseconds(50));
                        memcpy(&j, (msg + i*sizeof(unsigned short) + sizeof(char)), sizeof(unsigned short));
                        i++;
                    }
                    
                    if(FILE_SEND)
                        icmp_msg[0] = {FILE_DATA};
                    else if(FILENAME_SEND)
                        icmp_msg[0] = {FILE_NAME};
                    else
                        icmp_msg[0] = {MESSAGE};
                    
                    Socket::send(icmp_msg, ICMP_HEAD);
                    break;
                    
                case DONE_SENDING:
                    SENDING_FINNISHED = true;
                    _MSG_ = (char**)calloc(MAX_SIZE, sizeof(char*));
                    break;
                    
                case KEEP_ALIVE:
                    icmp_msg[0] = {YES_KEEP_ALIVE};
                    Socket::send(icmp_msg, ICMP_HEAD);
                    break;
                    
                case YES_KEEP_ALIVE:
                    this -> KEEPALIVE = true;
                    break;
                    
                default:
                    recive_data(msg);
                    break;
            }
    }while(true);
    
}

void Mattenger::send_file(const char* f_name, size_t size){
    
    Mattenger::send_msg(f_name, size, FILE_NAME, false);
    while(!SENDING_FINNISHED);
    
    std::ifstream infile;
    infile.open(f_name, std::ios::binary | std::ios::ate | std::ios::in);
    
    infile.seekg (0, infile.end);
    int N = infile.tellg();
    infile.seekg (0, infile.beg);
    char msg[N];
    infile.read(msg, N);
    Mattenger::send_msg(msg, N, FILE_DATA, false);
    printf("Súbor %s poslaný\n", f_name);
    
}

void Mattenger::start(){
    
    std::thread t1(&Mattenger::recive_msg, this);
    t1.detach();
    
    std::thread t2(&Mattenger::keep_alive, this);
    t2.detach();
    
}
