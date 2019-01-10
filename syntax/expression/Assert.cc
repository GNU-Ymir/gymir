#include <ymir/syntax/expression/Assert.hh>

namespace syntax {

    Assert::Assert () :
	_test (Expression::empty ()),
	_msg (Expression::empty ())
    {}

    Expression Assert::init (const Assert & asrt) {
	auto ret = new (Z0) Assert ();
	ret-> _location = asrt._location;
	ret-> _test = asrt._test;
	ret-> _msg = asrt._msg;
	return Expression {ret};
    }
    
    Expression Assert::init (const lexing::Word & loc, const Expression & test, const Expression & msg) {
	auto ret = new (Z0) Assert ();
	ret-> _location = loc;
	ret-> _test = test;
	ret-> _msg = msg;
	return Expression {ret};
    }

    Expression Assert::clone () const {
	return Assert::init (*this);
    }

    bool Assert::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Assert thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }	    
   
}
