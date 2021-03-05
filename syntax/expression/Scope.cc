#include <ymir/syntax/expression/Scope.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace syntax {

    Scope::Scope () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}

    Scope::Scope (const lexing::Word& loc, const Expression & content) :
	IExpression (loc),
	_content (content)
    {}
    
    Expression Scope::init (const lexing::Word & location, const Expression & content) {
	return Expression {new (NO_GC) Scope (location, content)};
    }

    bool Scope::isExit () const {
	return this-> getLocation ().getStr () == Keys::EXIT;
    }

    bool Scope::isSuccess () const {
	return this-> getLocation ().getStr () == Keys::SUCCESS;
    }

    bool Scope::isFailure () const {
	return this-> getLocation ().getStr () == Keys::FAILURE;
    }

    void Scope::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Scope> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _content.treePrint (stream, i + 1);
    }    

    const Expression& Scope::getContent () const {
	return this-> _content;
    }


    std::string Scope::prettyString () const {
	return Ymir::format ("% %", this-> getLocation ().getStr (), this-> _content.prettyString ());
    }
	
    const std::set <std::string> & Scope::computeSubVarNames () {
	this-> setSubVarNames (this-> _content.getSubVarNames ());
	return this-> getSubVarNames ();
    }
    
}
