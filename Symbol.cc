#include "semantic/pack/Symbol.hh"
#include "semantic/types/InfoType.hh"
#include <ymir/semantic/tree/Tree.hh>

namespace semantic {

    ulong ISymbol::__nbTmp__ = 0;
    
    ISymbol::ISymbol (Word word, InfoType type) :
	sym (word),
	type (type)
    {}
    
    bool ISymbol::isConst () {
	return this-> type-> isConst ();
    }

    void ISymbol::isConst (bool is) {
	this-> type-> isConst (is);
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
	return this-> type && this-> type-> value () != NULL;
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

    Value& ISymbol::value () {
	return this-> type-> value ();
    }
    
    std::string ISymbol::simpleTypeString () {
	if (this-> type-> isConst ())
	    return std::string ("c") + this-> type-> simpleTypeString ();
	return this-> type-> simpleTypeString ();
    }

    void ISymbol::resetNbTmp () {
	__nbTmp__ = 0;
    }

    ulong ISymbol::getLastTmp () {
	__nbTmp__ ++;
	return __nbTmp__;
    }
    
}
