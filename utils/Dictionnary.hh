#pragma once

#include <map>

namespace Ymir {
    

    class Dictionnary {		
    public :

	class Key {
	    ulong _value;
	    ulong _length;

	public :

	    Key (ulong val, ulong len);

	    Key ();

	    bool operator == (const Key & other) const ;

	    bool operator < (const Key & other) const ;

	    ulong length () const;
	    
	};
	
    private :

	static Dictionnary __instance__;
	
	std::map <std::string, Key> _index;
	std::map <Key, std::string> _reverse;

	ulong _lastId = 0;

	Dictionnary ();
	
    public :


	static Key insert (const std::string & text);

	static const std::string & get (Key key);
       
	
    };
    
}
