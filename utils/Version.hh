#pragma once

#include <string>
#include <set>

struct Version {

    static std::string DEBUG;
    // static std::string UNIX;// = "unix";
    // static std::string x84_64;// = "x86_64";
    // static std::string LINUX;// = "linux";
    // static std::string GNU_LINUX;// = "gnu_linux";
    // static std::string AMD64;// = "amd64";
    
    static std::set <std::string> __userDefs__;
    
    static bool isOn (const std::string& ver);

    static void addVersion (const std::string& ver);    

};



