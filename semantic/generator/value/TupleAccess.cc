#include <ymir/semantic/generator/value/TupleAccess.hh>

namespace semantic {

    namespace generator {

	TupleAccess::TupleAccess () :
	    _tuple (Generator::empty ())
	{
	    this-> isLvalue (true);
	}

	TupleAccess::TupleAccess (const lexing::Word & loc, const Generator & type, const Generator & tuple, uint index) :
	    Value (loc, type),
	    _tuple (tuple),
	    _index (index)
	{
	    this-> isLvalue (true);
	}


	Generator TupleAccess::init (const lexing::Word & loc, const Generator & type, const Generator & tuple, uint index) {
	    return Generator {new TupleAccess (loc, type, tuple, index)};
	}
	
	Generator TupleAccess::clone () const {
	    return Generator {new TupleAccess (*this)};
	}

	bool TupleAccess::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    TupleAccess thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}
	
	bool TupleAccess::equals (const Generator & gen) const {
	    if (!gen.is <TupleAccess> ()) return false;
	    auto bin = gen.to<TupleAccess> ();	    
	    return bin._tuple.equals (this-> _tuple) && bin._index == this-> _index;
	}

	const Generator & TupleAccess::getTuple () const {
	    return this-> _tuple;
	}

	uint TupleAccess::getIndex () const {
	    return this-> _index;
	}

	std::string TupleAccess::prettyString () const {
	    return Ymir::format ("%.%", this-> _tuple.prettyString (), this-> _index);
	}

    }    

}
