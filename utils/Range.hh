#pragma once
#include <stdint.h>

namespace Ymir {
    
    /**
     * A range iterator, used in for loops
     * implemented in utils/Range.cc
     */
    class rIterator {
    
	int64_t curr;
	bool rev;
	
    public:

	rIterator (int64_t, bool);
	
	int64_t operator* ();
	void operator++ ();
	bool operator!= (const rIterator&);
    
    };

    /**
     * A range value used in for loops
     * implemented in utils/Range.cc
     */
    class Range {
	int64_t _beg;
	int64_t _end;

    public:

	/**
	 * \param beg the value of the first element 
	 * \param end the value of the end
	 */
	Range (int64_t beg, int64_t end);
    
	const rIterator begin () const;

	const rIterator end () const ;

	int64_t fst ();

	int64_t scd ();
    
    };

    /**
     * \return a range between beg and end
     */
    Range r (int64_t beg, int64_t end);
    
}
