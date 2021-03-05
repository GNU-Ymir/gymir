#include <ymir/syntax/expression/Assert.hh>

namespace syntax {

    Assert::Assert () :
	IExpression (lexing::Word::eof ()),
	_test (Expression::empty ()),
	_msg (Expression::empty ())
    {}
    
    Assert::Assert (const lexing::Word & loc, const Expression & test, const Expression & msg) :
	IExpression (loc),
	_test (test),
	_msg (msg)
    {}

    Expression Assert::init (const lexing::Word & loc, const Expression & test, const Expression & msg) {
	return Expression {new (NO_GC) Assert (loc, test, msg)};
    }

    const Expression & Assert::getTest () const {
	return this-> _test;
    }

    const Expression & Assert::getMsg () const {
	return this-> _msg;
    }
    
    void Assert::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Assert> ", i, '\t');
	this-> _test.treePrint (stream, i + 1);
	this-> _msg.treePrint (stream, i + 1);
    }

    std::string Assert::prettyString () const {
	if (this-> _msg.isEmpty ())
	    return Ymir::format ("assert (%)", this-> _test.prettyString ());
	else 
	    return Ymir::format ("assert (%, %)", this-> _test.prettyString (), this-> _msg.prettyString ());
    }

    const std::set <std::string> & Assert::computeSubVarNames () {
	auto lSet = this-> _test.getSubVarNames ();
	auto & rSet = this-> _msg.getSubVarNames ();
	lSet.insert (rSet.begin (), rSet.end ());
	this-> setSubVarNames (lSet);
	return this-> getSubVarNames ();
    }
    
}
