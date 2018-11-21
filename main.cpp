//
//  main.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Mattenger.hpp"

int main() {
    
    Mattenger matt = Mattenger("192.168.0.104", 50050);
    matt.start();
    
    std::string msg;
    
    std::cout << "Napíš niečo:" << std::endl;
    while(true){
        std::getline(std::cin, msg);
        matt.send_msg(msg.c_str(), msg.size());
    }
    
    return 0;
}
