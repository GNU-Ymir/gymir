#include <ymir/syntax/declaration/Module.hh>

namespace syntax {
    
    Module::Module () :
	IDeclaration (lexing::Word::eof (), "")
    {}

    Module::Module (const lexing::Word & ident, const std::string & comment, const std::vector <Declaration> & decls, bool isGlobal) :
	IDeclaration (ident, comment),
	_decls (decls),
	_isGlobal (isGlobal)
    {}
    
    Declaration Module::init (const lexing::Word & ident, const std::string & comment, const std::vector <Declaration> & decls, bool isGlobal) {
	return Declaration {new (NO_GC) Module (ident, comment, decls, isGlobal)};
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
