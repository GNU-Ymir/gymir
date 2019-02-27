#pragma once

namespace Ymir {
    
    /**
     * A range iterator, used in for loops
     * implemented in utils/Range.cc
     */
    class rIterator {
    
	long curr;
	bool rev;
	
    public:

	rIterator (long, bool);
	
	long operator* ();
	void operator++ ();
	bool operator!= (const rIterator&);
    
    };

    /**
     * A range value used in for loops
     * implemented in utils/Range.cc
     */
    class Range {
	long _beg;
	long _end;

    public:

	/**
	 * \param beg the value of the first element 
	 * \param end the value of the end
	 */
	Range (long beg, long end);
    
	const rIterator begin () const;

	const rIterator end () const ;

	long fst ();

	long scd ();
    
    };

    /**
     * \return a range between beg and end
     */
    Range r (long beg, long end);
    
}
