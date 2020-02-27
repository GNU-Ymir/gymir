#include <ymir/syntax/declaration/Global.hh>

namespace syntax {


    Global::Global () :
	IDeclaration (lexing::Word::eof ()),
	_decl (Expression::empty ())
    {}

    Global::Global (const lexing::Word & loc, const Expression & expr) :
	IDeclaration (loc),
	_decl (expr)
    {}

    Declaration Global::init (const lexing::Word & location, const Expression & decl) {
	return Declaration {new (Z0) Global (location, decl)};
    }

    bool Global::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Global thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
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
