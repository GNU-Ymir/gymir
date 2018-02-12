#include <ymir/utils/Options.hh>

Options Options::__instance__;

bool & Options::isVerbose () {
    return this-> _isVerbose;
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
