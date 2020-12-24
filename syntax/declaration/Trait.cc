#include <ymir/syntax/declaration/Trait.hh>

namespace syntax {

    Trait::Trait () :
	IDeclaration (lexing::Word::eof ())
    {}

    Trait::Trait (const lexing::Word & name, const std::vector <Declaration> & decls) :
	IDeclaration (name),
	_inner (decls)
    {}
    
    Declaration Trait::init (const lexing::Word & name, const std::vector <Declaration> & decls) {
	return Declaration {new (NO_GC) Trait (name, decls)};
    }
    
    bool Trait::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Trait thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    
    
    void Trait::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Trait> ", i, '\t');
	stream.writeln (this-> getLocation ());

	for (auto & it : this-> _inner)
	    it.treePrint (stream, i + 1);
    }

    const std::vector<Declaration> & Trait::getDeclarations () const {
	return this-> _inner;
    }
    
}
