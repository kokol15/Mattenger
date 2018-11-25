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
bool SENDING_FINNISHED = true;
bool KEEPALIVE = true;
bool ALTER_CRC = false;
unsigned short FRAGMENT_SIZE = 60000;
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

unsigned short computeCRC(const char* s){
    
    unsigned short h = FIRST;
    while (*s) {
        h = (h * A) ^ (s[0] * B);
        s++;
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
        
        _crc_ = computeCRC((msg + HEAD));
        if(_crc_ != crc){
            std::cout << "Message has been altered" << std::endl;
            return;
        }
        
        MSG[seq_num] = (char*)calloc(size_num + 1, sizeof(char));
        memcpy(MSG[seq_num], (msg + HEAD), size_num*sizeof(char));
        
        MSG[seq_num][size_num] = 0;
    }
    
}

Mattenger::Mattenger(const char *addr){
    
    Socket::create_comm_point(addr);
    Socket::bind_socket();
    
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
    
    unsigned short i = 0, j = 0;
    std::string _resend_;
    std::string recreate_msg;
    
    for(j = 0; j < FRAG_TOTAL_NUM; j++){
        if(MSG[j] == NULL){
            unsigned short tmp = j;
            tmp++;
            memcpy((resend + i*sizeof(unsigned short)), &tmp, sizeof(unsigned short));
            i++;
        }
    }
    
    if(i > 0){
        _resend_.push_back(RESEND);
        i = 0;
        while(resend[i] != 0)
            _resend_.push_back(resend[i++]);
        _resend_.push_back(0);
        _resend_.push_back(0);
        _resend_.push_back(0);
        _resend_.push_back(0);
        
        Socket::send(_resend_.c_str(), _resend_.size());
        _resend_.clear();
        return "";
    }
    
    i = 0;
    while(MSG[i] != NULL)
        recreate_msg += MSG[i++];
    
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
            
            unsigned short i = 0, k = 0, j, _i;
            
            unsigned short num = size/FRAGMENT_SIZE;
            if(size % FRAGMENT_SIZE > 0)
                num++;
            
            char _msg_[HEAD + FRAGMENT_SIZE];
            memcpy((_msg_ + FRAGMENT_SIZE_INFO), &FRAGMENT_SIZE, sizeof(unsigned short));
            memcpy((_msg_ + FRAGMENT_NUM_INFO), &num, sizeof(unsigned short));
            
            while(i < num){
                _i = 0;
                j = HEAD;
                
                memcpy((_msg_ + FRAGMENT_SEQNUM_INFO), &i, sizeof(unsigned short));
                while( _i++ < FRAGMENT_SIZE && k != size) _msg_[j++] = msg[k++];
                
                _msg_[j++] = 0;
                //printf("%s|", (_msg_ + HEAD));
                
                unsigned short crc = computeCRC((_msg_ + HEAD));
                memcpy((_msg_ + FRAGMENT_CRC_INFO), &crc, sizeof(unsigned short));
                
                _MSG_[i] = (char*)calloc(1, sizeof(_msg_) + 1);
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
                    
                case DATA_END:
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
                    
                    icmp_msg[0] = {DATA_END};
                    Socket::send(icmp_msg, ICMP_HEAD);
                    break;
                    
                case DONE_SENDING:
                    SENDING_FINNISHED = true;
                    _MSG_ = (char**)calloc(MAX_SIZE, sizeof(char*));
                    MSG = (char**)calloc(MAX_SIZE, sizeof(char*));
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
    std::this_thread::sleep_for (std::chrono::milliseconds(50));
    
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
    
    /*std::cout << "Velkost posielaného fragmentu?" << std::endl;
    scanf("%hu", &FRAGMENT_SIZE);
    
    std::string choice = 0;
    std::cout << "Chces zaslat chybný fragment? [0 - NIE, 1 - ANO]" << std::endl;
    std::cin >> choice;
    if(choice == "1")
        ALTER_CRC = true;*/
    
    std::thread t1(&Mattenger::recive_msg, this);
    t1.detach();
    
    std::thread t2(&Mattenger::keep_alive, this);
    t2.detach();
    
    //Mattenger::send_file("hello.txt");
    
}
