#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>

namespace semantic {

    Symbol Symbol::__empty__ (Symbol::empty ());
    
    ISymbol::ISymbol () :
	_name (lexing::Word::eof ()),
	_referent (nullptr)
    {}

    ISymbol::ISymbol (const lexing::Word & name)
	: _name (name),
	  _referent (nullptr) 
    {}

    bool ISymbol::isOf (const ISymbol *) const {
	return false;
    }
    
    const lexing::Word & ISymbol::getName () const {
	return this-> _name;
    }
        
    void ISymbol::insert (const Symbol &) {
	Ymir::Error::halt (Ymir::ExternalError::get (Ymir::INSERT_NO_TABLE));
    }

    const Symbol & ISymbol::get (const std::string & name) const {
	return this-> getReferent ().get (name);
    }

    const Symbol& ISymbol::getReferent () const {
	if (this-> _referent == nullptr) return Symbol::__empty__;
	else return *(this-> _referent);
    }
    
    Symbol& ISymbol::getReferent () {
	if (this-> _referent == nullptr) return Symbol::__empty__;
	else return *(this-> _referent);
    }    
    
    ISymbol::~ISymbol () {}

    Symbol::Symbol (ISymbol * value) : Proxy<ISymbol, Symbol> (value)
    {}
    
    Symbol Symbol::empty () {
	return Symbol {nullptr};
    }

    bool Symbol::isEmpty () const {
	return this-> _value == nullptr;
    }

    const lexing::Word & Symbol::getName () const {	
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return this-> _value-> getName ();
    }

    void Symbol::insert (const Symbol & sym) {
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	this-> _value-> insert (sym);
    }

    const Symbol & Symbol::get (const std::string & name) const {
	if (this-> _value == nullptr)
	    return Symbol::__empty__;
	return this-> _value-> get (name);
    }
    
    
}
