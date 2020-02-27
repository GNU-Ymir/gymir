#include <ymir/syntax/declaration/Module.hh>

namespace syntax {
    
    Module::Module () :
	IDeclaration (lexing::Word::eof ())
    {}

    Module::Module (const lexing::Word & ident, const std::vector <Declaration> & decls, bool isGlobal) :
	IDeclaration (ident),
	_decls (decls),
	_isGlobal (isGlobal)
    {}
    
    Declaration Module::init (const lexing::Word & ident, const std::vector <Declaration> & decls, bool isGlobal) {
	return Declaration {new (Z0) Module (ident, decls, isGlobal)};
    }

    bool Module::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Module thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void Module::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Module>", i, '\t');
	stream.writeln (this-> getLocation ());

	for (auto & it : this-> _decls) {
	    it.treePrint (stream, i + 1);
	}
    }

    bool Module::isGlobal () const {
	return this-> _isGlobal;
    }

    const std::vector <Declaration> & Module::getDeclarations () const {
	return this-> _decls;
    }
    
}
