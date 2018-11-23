//
//  main.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Mattenger.hpp"

int main() {
    Mattenger matt = Mattenger("192.168.0.104");
    matt.start();
    
    std::string msg;
    short FRAGMENT_SIZE;
    char choice = 0;
    
    while(true){
        std::cout << "Aká má byt veľkosť fragmentu?" << std::endl;
        std::cin >> FRAGMENT_SIZE;
        std::cout << "Chces zaslat chybný fragment? [0 - NIE, 1 - ANO]" << std::endl;
        std::cout << "Napíš niečo:" << std::endl;
        std::getline(std::cin, msg);
        matt.send_msg(msg.c_str(), msg.size(), FRAGMENT_SIZE, choice);
    }
    
    return 0;
}

