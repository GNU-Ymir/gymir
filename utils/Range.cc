#include <ymir/utils/Range.hh>

namespace Ymir {

    rIterator::rIterator (long curr, bool rev) :
	curr (curr),
	rev (rev)
    {}

    long rIterator::operator* () {
	return this-> curr;
    }

    void rIterator::operator++ () {
	if (this-> rev) this-> curr--;
	else this-> curr ++;
    }

    bool rIterator::operator!= (const rIterator& ot) {
	return ot.curr != this-> curr;
    }

    Range::Range (long beg, long end) :
	_beg (beg),
	_end (end)
    {}

    const rIterator Range::begin () const {
	if (_beg > _end) return rIterator (_beg, true);
	else return rIterator (_beg, false);
    }
    
    const rIterator Range::end () const {
	return rIterator (_end, false);
    }

    long Range::fst () {
	return _beg;
    }
    
    long Range::scd () {
	return _end;
    }
   
    Range r (long beg, long end) {
	return {beg, end};
    }
    
}
