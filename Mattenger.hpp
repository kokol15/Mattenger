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
#define ICMP_HEAD sizeof(char)
#define HEAD (4 * sizeof(short))
#define RESEND_HEAD (sizeof(char) + sizeof(short))
#define SYN 'S'
#define SYN_ACK 'K'
#define ACK 'A'
#define DATA 'D'
#define DATA_END 'E'
#define RESEND 'R'
#define DONE_SENDING 'F'
#define KEEP_ALIVE 'X'
#define YES_KEEP_ALIVE 'Y'

class Mattenger: public Socket{
    
public:
    Mattenger(const char *);
    void send_msg(const char*, size_t);
    void recive_msg();
    void start();
    void keep_alive();
    
};

#endif /* Mattenger_hpp */
