#include <ymir/syntax/expression/While.hh>

namespace syntax {

    While::While () :
	IExpression (lexing::Word::eof ()),
	_test (Expression::empty ()),
	_content (Expression::empty ())
    {}    
    
    While::While (const lexing::Word & loc, const Expression & test, const Expression & content, bool isDo) :
	IExpression (loc),
	_test (test),
	_content (content),
	_isDo (isDo)
    {}

    Expression While::init (const lexing::Word & location, const Expression & test, const Expression & content, bool isDo) {
	return Expression {new (NO_GC) While (location, test, content, isDo)};
    }

    void While::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<While>", i, '\t');
	this-> _test.treePrint (stream, i + 1);
	this-> _content.treePrint (stream, i + 1);
    }

    const Expression & While::getTest () const {
	return this-> _test;
    }

    const Expression & While::getContent () const {
	return this-> _content;
    }

    bool While::isDo () const {
	return this-> _isDo;
    }

    std::string While::prettyString () const {
	if (this-> _isDo) {
	    return Ymir::format ("do % while (%)", this-> _content.prettyString (), this-> _test.prettyString ());
	} else {
	    return Ymir::format ("while (%) %", this-> _test.prettyString (), this-> _content.prettyString ());
	}
    }
    
    const std::set <std::string> & While::computeSubVarNames () {
	auto tSet = this-> _test.getSubVarNames ();
	auto & cSet = this-> _content.getSubVarNames ();
	tSet.insert (cSet.begin (), cSet.end ());
	this-> setSubVarNames (tSet);
	return this-> getSubVarNames ();
    }
    
}
