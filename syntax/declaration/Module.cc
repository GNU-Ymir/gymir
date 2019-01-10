#include <ymir/syntax/declaration/Module.hh>

namespace syntax {

    const std::string Module::KEYWORD = "mod";
    
    Module::Module () :
	_constructor (Declaration::empty ()),
	_destructor (Declaration::empty ())
    {}
    
    Declaration Module::init () {
	return Declaration {new (Z0) Module ()};
    }

    Declaration Module::init (const Module & mod) {
	auto ret = new (Z0) Module ();
	ret-> _ident = mod. _ident;
	ret-> _decls = mod. _decls;
	ret-> _isGlobal = mod. _isGlobal;
	return Declaration {ret};
    }

    Declaration Module::init (const lexing::Word & ident, const std::vector <Declaration> & decls) {
	auto ret = new (Z0) Module ();
	ret-> _ident = ident;
	ret-> _decls = decls;
	ret-> _isGlobal = false;
	return Declaration {ret};
    }

    Declaration Module::clone () const {
	return Module::init (*this);
    }

    bool Module::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Module thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    
    
    void Module::setIdent (const lexing::Word & ident) {
	this-> _ident = ident;	
    }

    void Module::addDeclaration (const Declaration& decl) {
	this-> _decls.push_back (decl);
    }

    void Module::isGlobal (bool is) {
	this-> _isGlobal = is;
    }
    
}
