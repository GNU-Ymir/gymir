#pragma once

#include <vector>
#include <string>


class Options {

    std::vector <std::string> _includeDirs;
    std::string _prefixIncludeDir;
    bool _isVerbose = false;
    bool _isDebug = false;
    
    static Options __instance__;
    
public:

    static Options & instance () {
	return __instance__;
    }
    
    bool & isVerbose ();

    bool & isDebug ();
    
    std::string & prefixIncludeDir ();

    void setPrefix (const char * path);

    std::vector <std::string> & includeDirs ();

    void addIncludeDir (const char * path);
};
