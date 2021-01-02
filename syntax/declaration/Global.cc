#include <ymir/syntax/declaration/Global.hh>

namespace syntax {


    Global::Global () :
	IDeclaration (lexing::Word::eof (), ""),
	_decl (Expression::empty ())
    {}

    Global::Global (const lexing::Word & loc, const std::string & comment, const Expression & expr) :
	IDeclaration (loc, comment),
	_decl (expr)
    {}

    Declaration Global::init (const lexing::Word & location, const std::string & comment, const Expression & decl) {
	return Declaration {new (NO_GC) Global (location, comment, decl)};
    }

    const Expression & Global::getContent () const {
	return this-> _decl;
    }
    
    void Global::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Global>", i, '\t');
	stream.writeln (" ", this-> getLocation ());       
	this-> _decl.treePrint (stream, i + 2);
    }
        
}
