#include <ymir/utils/Options.hh>

Options Options::__instance__;

bool & Options::isVerbose () {
    return this-> _isVerbose;
}

bool & Options::isDebug () {
    return this-> _isDebug;
}

bool & Options::isStandalone () {
    return this-> _isStandalone;
}

void Options::setExecutable (const char * path) {
    this-> _executable = path;
    auto index = this-> _executable.find_last_of ("/");
    this-> setPrefix (this-> _executable.substr (0, index).c_str ());    
}

void Options::setPrefix (const char * path) {
    this-> _prefixIncludeDir = path;
    if (this-> _prefixIncludeDir [this-> _prefixIncludeDir.length () - 1] != '/')
	this-> _prefixIncludeDir += "/include/";
    else
	this-> _prefixIncludeDir += "include/";
}

std::string & Options::prefixIncludeDir () {
    return this-> _prefixIncludeDir;
}

std::vector <std::string> & Options::includeDirs () {
    return this-> _includeDirs;
}

void Options::addIncludeDir (const char * path) {
    this-> _includeDirs.push_back (path);
    if (this-> _includeDirs.back () [this-> _includeDirs.back ().length () - 1] != '/')
	this-> _includeDirs.back () += "/";
}

void Options::lintTime () {
    this-> _current = LINT;    
}

void Options::semanticTime () {
    this-> _current = SEMANTIC;
}

bool Options::itsLintTime () {
    return this-> _current == LINT;
}
