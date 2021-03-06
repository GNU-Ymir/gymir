#include <ymir/utils/Dictionnary.hh>

namespace Ymir {

    Dictionnary Dictionnary::__instance__;
    
    Dictionnary::Dictionnary () {
	this-> _index [""] = Key (0, 0);
	this-> _reverse [Key (0, 0)] = "";
    }
    
    Dictionnary::Key Dictionnary::insert (const std::string & text) {
	auto it = __instance__._index.find (text);
	if (it != __instance__. _index.end ()) {
	    return it-> second;
	} else {
	    __instance__. _lastId += 1;
	    __instance__. _index [text] = Key (__instance__. _lastId, text.length ());
	    __instance__. _reverse [Key (__instance__. _lastId, text.length ())] = text;
	    return __instance__._lastId;
	}
    }

    const std::string & get (Dictionnary::Key key) {
	auto & it = __instance__._reverse.find (key);
	if (it != __instance__. _reverse.end ())
	    return it-> second;
	else return __instance__._reverse [Key (0, 0)];
    }

    Dictionnary::Key::Key (ulong v, ulong l) :
	_value (v),
	_len (l)	
    {}

    Dictionnary::Key::Key () :
	_value (0), _len (0)
    {}

    bool Dictionnary::Key::operator== (const Dictionnary::Key &other) const {
	return this-> _value == other._value;
    }

    bool Dictionnary::Key::operator < (const Dictionnary::Key &other) const {
	return this-> _value < other._value;
    }

    ulong Dictionnary::Key::length () const {
	return this-> _len;
    }
    
    
}
