//
//  main.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Mattenger.hpp"

int main() {
    Mattenger matt = Mattenger("172.20.10.5");
    matt.start();
    
    std::string msg;
    
    std::cout << "Napíš niečo:" << std::endl;
    
    while(true){
        std::getline(std::cin, msg);
        matt.send_msg(msg.c_str(), msg.size());
    }
    
    return 0;
}
