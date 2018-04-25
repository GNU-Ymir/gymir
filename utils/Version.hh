#pragma once

#include <string>

struct Version {

    static std::string DEBUG;
    static std::string UNIX;// = "unix";
    static std::string x84_64;// = "x86_64";
    static std::string LINUX;// = "linux";
    static std::string GNU_LINUX;// = "gnu_linux";
    static std::string AMD64;// = "amd64";
    
    static bool isOn (const std::string& ver);
    

};



