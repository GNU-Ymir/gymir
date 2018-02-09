#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/syntax/Word.hh>
#include <cstring>


namespace Ymir {
    
    void OutBuffer::write_ (const char* cs) {
	auto cslen = strlen (cs);
	if (capacity < len + cslen) {
	    resize (len + cslen);
	}

	for (uint i = len, j = 0 ; j < cslen ; i++, j++) {
	    this-> current [i] = cs [j]; 
	}
	
	len += cslen;
    }

    
    void OutBuffer::write_ (const std::string& cs) {
	if (capacity < len + cs.length ()) {
	    resize (len + cs.length ());
	}
	
	for (uint i = len, j = 0 ; j < cs.length () ; i++, j++) {
	    this-> current [i] = cs [j]; 
	}
	
	len += cs.length ();
    }
    
    void OutBuffer::write_ (ulong nb) {
	auto len = snprintf (NULL, 0, "%lu", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + this-> len, "%lu", nb);
	this-> len += len;
    }

    void OutBuffer::write_ (long nb) {
	auto len = snprintf (NULL, 0, "%ld", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + this-> len, "%ld", nb);
	this-> len += len;
    }


    void OutBuffer::write_ (int nb) {
	auto len = snprintf (NULL, 0, "%d", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + this-> len, "%d", nb);
	this-> len += len;
    }

    
    void OutBuffer::write_ (double nb) {
	auto len = snprintf (NULL, 0, "%A", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + this-> len, "%A", nb);
	this-> len += len;
    }
    
    void OutBuffer::write_ (char c) {
	if (this-> capacity < this-> len + 1) {
	    resize (this-> len + 1);
	}

	this-> current [this-> len] = c;
	len += 1;
    }
    
    void OutBuffer::write_ (syntax::Expression expr) {
	if (expr)
	    this-> write (expr-> prettyPrint ().c_str ());
	else
	    this-> write ("null");
    }

    void OutBuffer::write_ (semantic::InfoType info) {
	if (info) 
	    this-> write (info-> typeString ().c_str ());
	else this-> write ("null");
    }
    
    void OutBuffer::write_ (const Word & word) {
	this-> write (word.toString ());
    }

    void OutBuffer::write_ (bool b) {
	if (b)
	    this-> write_ ("true");
	else this-> write_ ("false");
    }

    void OutBuffer::write_ (float nb) {
	auto len = snprintf (NULL, 0, "%A", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + this-> len, "%A", nb);
	this-> len += len;
    }
    
    void OutBuffer::resize (ulong len) {
	if (capacity == 0) capacity = len + 1;
	else if (capacity * 2 < len) capacity = len + capacity + 1;
	else capacity = (capacity * 2) + 1;
	
	auto aux = new (Z0) char [capacity];

	for (uint i = 0 ; i < this-> len ; i ++)
	    aux [i] = this-> current [i];
	
	delete[] this-> current;
	this-> current = aux;
    }
    
    void OutBuffer::write () {}

    
    void OutBuffer::throwError () {
	Ymir::Error::assert ("error");
    }
    
}
