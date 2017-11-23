#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>
#include <cstring>

namespace Ymir {
    
    void OutBuffer::write_ (const char* cs) {
	auto cslen = strlen (cs);
	if (capacity < len + cslen) {
	    resize (len + cslen);
	}

	for (int i = len, j = 0 ; j < cslen ; i++, j++) {
	    this-> current [i] = cs [j]; 
	}
	
	len += cslen;
    }

    
    void OutBuffer::write_ (std::string cs) {
	if (capacity < len + cs.length ()) {
	    resize (len + cs.length ());
	}
	
	for (int i = len, j = 0 ; j < cs.length () ; i++, j++) {
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
	auto len = snprintf (NULL, 0, "%lf", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + this-> len, "%lf", nb);
	this-> len += len;
    }
    
    void OutBuffer::write_ (char c) {
	if (this-> capacity < this-> len + 1) {
	    resize (this-> len + 1);
	}

	this-> current [this-> len] = c;
	len += 1;
    }
    
    void OutBuffer::resize (ulong len) {
	if (capacity == 0) capacity = len + 1;
	else if (capacity * 2 < len) capacity = len + capacity + 1;
	else capacity = (capacity * 2) + 1;
	
	auto aux = new char [capacity];

	for (int i = 0 ; i < this-> len ; i ++)
	    aux [i] = this-> current [i];
	
	delete[] this-> current;
	this-> current = aux;
    }


    void OutBuffer::write () {}

}
