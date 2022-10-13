#include <ymir/utils/Range.hh>

namespace Ymir {

    rIterator::rIterator (int64_t curr, bool rev) :
	curr (curr),
	rev (rev)
    {}

    int64_t rIterator::operator* () {
	return this-> curr;
    }

    void rIterator::operator++ () {
	if (this-> rev) this-> curr--;
	else this-> curr ++;
    }

    bool rIterator::operator!= (const rIterator& ot) {
	return ot.curr != this-> curr;
    }

    Range::Range (int64_t beg, int64_t end) :
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

    int64_t Range::fst () {
	return _beg;
    }
    
    int64_t Range::scd () {
	return _end;
    }
   
    Range r (int64_t beg, int64_t end) {
	return {beg, end};
    }
    
}
