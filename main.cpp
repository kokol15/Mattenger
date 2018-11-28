//
//  main.cpp
//  Mattenger
//
//  Created by Matej on 02/11/2018.
//  Copyright © 2018 Matej. All rights reserved.
//

#include "Mattenger.hpp"

int main() {
    bool bad_frag = false;
    
    std::string alter_msg;
    int port, frag_size;
    
    std::cout << "Zadaj číslo portu:" << std::endl;
    std::cin >> port;
    
    std::cout << "Zadaj veľkosť fragmentu:" << std::endl;
    std::cin >> frag_size;

    std::cout << "Chces zaslať chybný rámec [ano/nie]?:" << std::endl;
    std::cin >> alter_msg;
    
    if(alter_msg == "ano")
        bad_frag = true;
    
    Mattenger matt = Mattenger("192.168.0.104", port, frag_size);
    matt.start();
    
    std::string f_name = "dveLaveRuky.JPG";
    matt.send_file(f_name.c_str(), f_name.size());
    
    std::string msg;
    
    std::cout << "Napíš niečo:" << std::endl;
    
    while(true){
        std::getline(std::cin, msg);
        matt.send_msg(msg.c_str(), msg.size(), MESSAGE, bad_frag);
    }
    
    return 0;
}
