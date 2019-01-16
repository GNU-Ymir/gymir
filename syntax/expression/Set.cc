#include <ymir/syntax/expression/Set.hh>

namespace syntax {

    Set::Set () {}

    Expression Set::init (const Set & op) {
	auto ret = new (Z0) Set ();
	ret-> _params = op._params;
	return Expression {ret};
    }

    Expression Set::init (const std::vector <Expression> & params) {
	auto ret = new (Z0) Set ();
	ret-> _params = params;
	return Expression {ret};
    }

    Expression Set::clone () const {
	return Set::init (*this);
    }

    bool Set::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Set thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Set::treePrint (Ymir::OutBuffer & stream, int i) const {
	for (auto & it : this-> _params)
	    it.treePrint (stream, i);
    }
    
}
