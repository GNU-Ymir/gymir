#include <ymir/syntax/expression/If.hh>

namespace syntax {
        
    If::If () :
	IExpression (lexing::Word::eof ()),
	_test (Expression::empty ()),
	_content (Expression::empty ()),
	_else (Expression::empty ())
    {}

    If::If (const lexing::Word & loc, const Expression & test, const Expression & content, const Expression & elsePart) :
	IExpression (loc),
	_test (test),
	_content (content),
	_else (elsePart)
    {}
    
   
    Expression If::init (const lexing::Word & location, const Expression & test, const Expression & content, const Expression & elsePart) {
	return Expression {new (NO_GC) If (location, test, content, elsePart)};
    }

    void If::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<If> ", i, '\t');
	this-> _test.treePrint (stream, i + 1);
	this-> _content.treePrint (stream, i + 1);
	this-> _else.treePrint (stream, i + 1);
    }

    const Expression & If::getTest () const {
	return this-> _test;
    }

    const Expression & If::getContent () const {
	return this-> _content;
    }

    const Expression & If::getElsePart () const {
	return this-> _else;
    }

    std::string If::prettyString () const {
	Ymir::OutBuffer buf;
	buf.writef ("if (%) % ", this-> _test.prettyString (), this-> _content.prettyString ());
	if (!this-> _else.isEmpty ()) buf.writef ("\nelse %", this-> _else.prettyString ());
	return buf.str ();
    }

    const std::set <std::string> & If::computeSubVarNames () {
	auto tSet = this-> _test.getSubVarNames ();
	auto & cSet = this-> _content.getSubVarNames ();
	auto & eSet = this-> _else.getSubVarNames ();
	tSet.insert (cSet.begin (), cSet.end ());
	tSet.insert (eSet.begin (), eSet.end ());
	this-> setSubVarNames (tSet);
	return this-> getSubVarNames ();
    }
	
    
}
