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
#define HEAD (sizeof(char) + (4 * sizeof(short)))
#define RESEND_HEAD (sizeof(char) + sizeof(short))
#define SYN 'S'
#define SYN_ACK 'K'
#define ACK 'A'
#define MESSAGE 'M'
#define DATA_END 'E'
#define RESEND 'R'
#define DONE_SENDING 'F'
#define KEEP_ALIVE 'X'
#define YES_KEEP_ALIVE 'Y'
#define FILE_NAME 'N'

class Mattenger: public Socket{
private:
    bool KEEPALIVE = true;
    
public:
    Mattenger(const char *);
    void send_msg(const char*, size_t, char);
    void recive_msg();
    void start();
    void keep_alive();
    void send_file(const char*);
    
};

#endif /* Mattenger_hpp */
