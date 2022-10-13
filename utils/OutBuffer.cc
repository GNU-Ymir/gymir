#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/Memory.hh>
#include <ymir/lexing/Word.hh>
#include <cstring>
#include <ymir/syntax/Expression.hh>
#include <ymir/semantic/Generator.hh>
#include <ymir/semantic/Symbol.hh>

namespace Ymir {    
   

    void OutBuffer::write_ (const char* cs) {
	auto cslen = strlen (cs);
	if (capacity < len + cslen) {
	    resize (len + cslen);
	}

	uint32_t i, j;
	for (i = len, j = 0 ; j < cslen ; i++, j++) {
	    this-> current [i] = cs [j];
	}
	this-> len += cslen;
    }
    
    void OutBuffer::write_ (const std::string& cs) {
	if (capacity <= len + cs.length ()) {
	    resize (len + cs.length ());
	}
	
	for (uint32_t i = len, j = 0 ; j < cs.length () ; i++, j++) {
	    this-> current [i] = cs [j]; 
	}
	
	len += cs.length ();
    }
    
    void OutBuffer::write_ (uint64_t nb) {
	auto len = snprintf (NULL, 0, "%llu", nb);
	if (this-> capacity <= this-> len + len) {
	    resize (this-> len + len);
	}

	snprintf (this-> current.data () + this-> len, (this-> capacity - this-> len), "%llu", nb);       	
	this-> len += len;
    }

    void OutBuffer::write_ (int64_t nb) {
	auto len = snprintf (NULL, 0, "%lld", nb);
	if (this-> capacity <= this-> len + len) {
	    resize (this-> len + len);
	}

	snprintf (this-> current.data () + this-> len, (this-> capacity - this-> len), "%lld", nb);
	this-> len += len;
    }


    void OutBuffer::write_ (int32_t nb) {
	auto len = snprintf (NULL, 0, "%d", nb);
	if (this-> capacity <= this-> len + len) {
	    resize (this-> len + len);
	}

	snprintf (this-> current.data () + this-> len, (this-> capacity - this-> len), "%d", nb);
	this-> len += len;
    }
    
    void OutBuffer::write_ (uint32_t nb) {
	auto len = snprintf (NULL, 0, "%d", nb);
	if (this-> capacity <= this-> len + len) {
	    resize (this-> len + len);
	}

	snprintf (this-> current.data () + this-> len, (this-> capacity - this-> len), "%d", nb);
	this-> len += len;
    }

    void OutBuffer::write_ (uint8_t nb) {
	auto len = snprintf (NULL, 0, "%x", nb);
	if (this-> capacity <= this-> len + len) {
	    resize (this-> len + len);
	}

	snprintf (this-> current.data () + this-> len, (this-> capacity - this-> len), "%x", nb);
	this-> len += len;
    }    

    void OutBuffer::write_ (__int128_t nb) {
	if (nb < 0) {
	    int64_t n = nb;
	    write_ (n);
	} else {
	    uint64_t n = nb;
	    write_ (n);
	}
    }
    
    void OutBuffer::write_ (double nb) {
	auto len = snprintf (NULL, 0, "%A", nb);
	if (this-> capacity <= this-> len + len) {
	    resize (this-> len + len);
	}

	snprintf (this-> current.data () + this-> len, (this-> capacity - this-> len), "%A", nb);
	this-> len += len;
    }
    
    void OutBuffer::write_ (char c) {
	if (this-> capacity <= this-> len + 1) {
	    resize (this-> len + 1);
	}
	
	this-> current [this-> len] = c;
	this-> len += 1;	
    }      
    
    void OutBuffer::write_ (const lexing::Word & word) {
	this-> write (word.toString ());
    }

    void OutBuffer::write_ (bool b) {
	if (b)
	    this-> write_ ("true");
	else this-> write_ ("false");
    }

    void OutBuffer::write_ (float nb) {
	auto len = snprintf (NULL, 0, "%f", nb);
	if (this-> capacity <= this-> len + len) {
	    resize (this-> len + len);
	}

	snprintf (this-> current.data () + this-> len, (this-> capacity - this-> len), "%f", nb);
	this-> len += len;
    }
    
    void OutBuffer::resize (uint64_t len) {
	if (capacity == 0) capacity = len + 1;
	else if (capacity * 2 < len) capacity = len + capacity + 1;
	else capacity = (capacity * 2) + 1;
	
	this-> current.resize (capacity, '\0');

	// for (uint i = 0 ; i < this-> len ; i ++)
	//     aux.push_back (this-> current [i]);
		
	// this-> current = aux;	
    }
    
    void OutBuffer::write () {}

    void OutBuffer::write_ (const syntax::Expression & expr) {
	this-> write (expr.prettyString ());
    }

    void OutBuffer::write_ (const semantic::generator::Generator & gen) {
	this-> write (gen.prettyString ());
    }

    void OutBuffer::write_ (const semantic::Symbol & sym) {
	this-> write (sym.getRealName ());
    }
    
    void OutBuffer::throwError () {
	//Ymir::Error::assert ("error");
    }

    std::string entab (const std::string & value) {
	Ymir::OutBuffer buf;
	uint64_t i = 0;
	for (auto & c : value) {
	    buf.write (c);
	    i += 1;
	    
	    if (c == '\n' && i != value.size ()) buf.write ('\t');
	    else if (c == '\r' && i != value.size ()) buf.write ('\t');
	}
	return buf.str ();
    }

    OutBuffer::~OutBuffer () {}
}
