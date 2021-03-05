#include <ymir/syntax/expression/ClassVar.hh>

namespace syntax {

    ClassVar::ClassVar () :
	IExpression (lexing::Word::eof ())
    {}
    
    ClassVar::ClassVar (const lexing::Word & loc) :
	IExpression (loc)
    {}
        
    Expression ClassVar::init (const lexing::Word & location) {
	auto ret = new (NO_GC) ClassVar (location);
	return Expression {ret};
    }

    void ClassVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<ClassVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
    }

    std::string ClassVar::prettyString () const {
	return "class " + this-> getLocation ().getStr ();
    }

    const std::set <std::string> & ClassVar::computeSubVarNames () {
	this-> setSubVarNames ({this-> getLocation ().getStr ()});
	return this-> getSubVarNames ();
    }
}
