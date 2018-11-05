#pragma once

#include <vector>
#include <string>

enum CurrentTime {
    LINT,
    SEMANTIC
};

class Options {

    std::vector <std::string> _includeDirs;
    std::string _prefixIncludeDir;
    std::string _executable;
    std::string _docFileName = "docs.json";
    bool _isVerbose = false;
    bool _isDebug = false;
    bool _isStandalone = false;
    bool _needDocs = false;
    
    static Options __instance__;
    CurrentTime _current;
    
public:

    static Options & instance () {
	return __instance__;
    }
    
    bool & isVerbose ();

    bool & isDebug ();

    bool & isStandalone ();

    bool&  generateDocs () ;

    std::string & docFileName ();
    
    std::string & prefixIncludeDir ();

    void setPrefix (const char * path);

    void setExecutable (const char * path);
    
    std::vector <std::string> & includeDirs ();

    void addIncludeDir (const char * path);
        
    void lintTime ();

    void semanticTime ();

    bool itsLintTime ();
    
};
