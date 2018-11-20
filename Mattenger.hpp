//
//  Mattenger.hpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#ifndef Mattenger_hpp
#define Mattenger_hpp

#include "Socket.hpp"
#define ICMP_HEAD 1
#define HEAD 10
#define SYN 'S'
#define SYN_ACK 'K'
#define ACK 'A'

class Mattenger: public Socket{
    
public:
    Mattenger(const char *, int);
    void send_msg(const char*, size_t);
    void recive_msg();
    void start();
    void handshake();
    void listen_for_connection();
};

#endif /* Mattenger_hpp */
