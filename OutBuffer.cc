#include <ymir/utils/OutBuffer.hh>

namespace Ymir {

    void OutBuffer::write (std::string cs) {
	if (capacity < len + cs.length ()) {
	    resize (len + cs.length ());
	}
	
	for (int i = len, j = 0 ; j < cs.length () ; i++, j++) {
	    this-> current [i] = cs [j]; 
	}
	
	len += cs.length ();
    }
    
    void OutBuffer::write (ulong nb) {
	auto len = snprintf (NULL, 0, "%lu", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + len, "%lu", nb);
    }

    void OutBuffer::write (long nb) {
	auto len = snprintf (NULL, 0, "%ld", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + len, "%ld", nb);
	this-> len += len;
    }

    void OutBuffer::write (double nb) {
	auto len = snprintf (NULL, 0, "%lf", nb);
	if (this-> capacity < this-> len + len) {
	    resize (this-> len + len);
	}

	sprintf (this-> current + len, "%lf", nb);
	this-> len += len;
    }
    
    void OutBuffer::write (char c) {
	if (this-> capacity < this-> len + 1) {
	    resize (this-> len + 1);
	}

	this-> current [len] = c;
	len += 1;
    }
    
    void OutBuffer::resize (ulong len) {
	if (capacity == 0) capacity = len + 1;
	else if (capacity * 2 < len) capacity = len + capacity + 1;
	else capacity = (capacity * 2) + 1;
	
	auto aux = new char [capacity];

	for (int i = 0 ; i < this-> len ; i ++)
	    aux [i] = this-> current [i];	
    }


    void OutBuffer::write () {}

}
