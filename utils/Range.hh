#pragma once

namespace Ymir {

    class rIterator {
    
	long curr;
	bool rev;
	
    public:

	rIterator (long, bool);
    
	long operator* ();
	void operator++ ();
	bool operator!= (const rIterator&);
    
    };

    class Range {
	long _beg;
	long _end;

    public:

	Range (long beg, long end);
    
	const rIterator begin () const;

	const rIterator end () const ;

	long fst ();

	long scd ();
    
    };

    Range r (long beg, long end);
    
}
