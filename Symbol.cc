#include "semantic/pack/Symbol.hh"
#include "semantic/types/InfoType.hh"
#include <ymir/semantic/tree/Tree.hh>

namespace semantic {

    ISymbol::ISymbol (Word word, InfoType type) :
	sym (word),
	type (type),
	value (NULL)
    {}
    
    bool ISymbol::isConst () {
	return this-> type-> isConst ();
    }

    bool& ISymbol::isStatic () {
	return this-> _static;
    }
    
    bool& ISymbol::isPublic () {
	return this-> _public;
    }

    bool ISymbol::isScoped () {
	return this-> scoped;
    }

    bool ISymbol::isImmutable () {
	return this-> value != NULL;
    }

    void ISymbol::quit (Namespace) {
    }

    std::string ISymbol::typeString () {
	return this-> type-> typeString ();
    }

    void ISymbol::treeDecl (Ymir::Tree tree) {
	this-> decl = tree;
    }

    Ymir::Tree ISymbol::treeDecl () {
	return this-> decl;
    }
    
    bool ISymbol::isType () {
	if (this-> type)
	    return this-> type-> isType ();
	return false;
    }
    
    std::string ISymbol::simpleTypeString () {
	if (this-> type-> isConst ())
	    return std::string ("c") + this-> type-> simpleTypeString ();
	return this-> type-> simpleTypeString ();
    }
    
}
