#include "semantic/pack/Symbol.hh"
#include "semantic/types/InfoType.hh"
#include <ymir/semantic/value/Value.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/semantic/pack/Table.hh>

namespace semantic {

    ulong ISymbol::__nbTmp__ = 0;

    DeclSymbol DeclSymbol::init () {
	return DeclSymbol {Table::instance ().getCurrentLifeTime (), 0};
    }    
    
    ISymbol::ISymbol (Word word, DeclSymbol sym, syntax::Expression from, InfoType type) :
	_space (""),
	_type (type),
	sym (word),
	_from (from)
    {
	this-> isPublic () = true;
	if (this-> _type)
	    this-> _type-> symbol () = this;
	this-> _decl = sym;
    }

    ISymbol::ISymbol (Word word, syntax::Expression from, InfoType type) :
	_space (""),
	_type (type),
	sym (word),
	_from (from)
    {
	this-> isPublic () = true;
	if (this-> _type)
	    this-> _type-> symbol () = this;
	this-> _decl = DeclSymbol::init ();
    }

    
    bool ISymbol::isConst () {
	return this-> _type-> isConst ();
    }

    void ISymbol::isConst (bool is) {
	this-> _type-> isConst (is);
    }

    bool& ISymbol::isStatic () {
	return this-> _static;
    }

    bool& ISymbol::isClosured () {
	return this-> _isClosured;
    }
    
    ulong ISymbol::lifeTime () {
	return this-> _decl.lifetime;
    }

    ulong& ISymbol::closureLifeTime () {
	return this-> _decl.closureLifeTime;
    }

    DeclSymbol ISymbol::getDeclSym () {
	return this-> _decl;
    }
    
    bool& ISymbol::isInline () {
	return this-> _isInline;
    }
    
    bool& ISymbol::isPublic () {
	return this-> _public;
    }

    bool ISymbol::isScoped () {
	return this-> scoped;
    }

    bool ISymbol::isImmutable () {
	return this-> _type &&
	    (this-> _type-> value () != NULL ||
	     this-> _type-> isImmutable ());
    }

    void ISymbol::quit (Namespace) {
    }

    InfoType ISymbol::type () {	
	return this-> _type;
    }

    void ISymbol::type (InfoType type) {
	if (this-> _type)
	    this-> _type-> symbol () = NULL;
	
	this-> _type = type;
	if (this-> _type)
	    this-> _type-> symbol () = this;
    }
    
    bool ISymbol::isLvalue () {
	if (this-> _from)
	    return this-> _from-> isLvalue ();
	return true; // Si il n'y a pas d'expression associé, c'est du built-in donc lvalue
    }
    
    std::string ISymbol::typeString () {
	return this-> _type-> typeString ();
    }

    void ISymbol::treeDecl (Ymir::Tree tree) {
	this-> decl = tree;
    }

    Ymir::Tree ISymbol::treeDecl () {
	return this-> decl;
    }
    
    bool ISymbol::isType () {
	if (this-> _type)
	    return this-> _type-> isType ();
	return false;
    }

    Value& ISymbol::value () {
	return this-> _type-> value ();
    }
    
    std::string ISymbol::simpleTypeString () {
	if (this-> _type-> isConst ())
	    return std::string ("c") + this-> _type-> simpleTypeString ();
	return this-> _type-> simpleTypeString ();
    }

    Namespace & ISymbol::space () {
	return this-> _space;
    }
    
    void ISymbol::resetNbTmp () {
	__nbTmp__ = 0;
    }

    ulong ISymbol::getLastTmp () {
	__nbTmp__ ++;
	return __nbTmp__;
    }
    
    ISymbol::~ISymbol () {
	if (this-> value () != NULL) {
	    delete this-> value ();
	    this-> value () = NULL;
	}
	
	if (this-> _type != NULL) {
	    delete this-> _type;
	    this-> _type = NULL;
	}
    }

}
