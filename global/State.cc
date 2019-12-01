#include <ymir/global/State.hh>
#include <ymir/utils/Path.hh>
#include <ymir/utils/OutBuffer.hh>
#include <algorithm>

namespace global {
    
    State::State () {}

    State::State (const State &) {}

    State & State::operator= (const State &) {
	return *this;
    }

    State & State::instance () {
	static State __instance__;
	return __instance__;
    }    

    bool State::isVersionActive (const std::string & version) {
	return std::find (this-> _activeVersion.begin (), this-> _activeVersion.end (), version) != this-> _activeVersion.end ();
    }

    void State::activateVersion (const std::string & version) {
	if (!isVersionActive (version))
	    this-> _activeVersion.push_back (version);
    }

    void State::addIncludeDir (const std::string & dir) {
	this-> _includeDir.emplace (dir);
    }

    const std::set <std::string> & State::getIncludeDirs () const {
	return this-> _includeDir;
    }

    void State::activateVerbose (bool verb) {
	this-> _isVerbose = verb;
    }

    bool State::isVerboseActive () const {
	return this-> _isVerbose;
    }

    void State::activateDebug (bool debug) {
	this-> _isDebug = debug;
    }

    bool State::isDebugActive () const {
	return this-> _isDebug;
    }

    void State::setPrefix (const std::string & path) {
	this-> _prefixPath = path;
	this-> _includeDir.emplace (Ymir::Path::build (path, __includeInPrefix__).toString ());	
	this-> _corePath = Ymir::Path::build (Ymir::Path::build (path, __includeInPrefix__), "core").toString ();
    }

    const std::string & State::getCorePath () const {
	return this-> _corePath;
    }

    const std::string & State::getPrefix () const {
	return this-> _prefixPath;
    }
    
}
