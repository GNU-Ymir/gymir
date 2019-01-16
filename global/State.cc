#include <ymir/global/State.hh>
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
    
}