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

    std::vector <Symbol> ISymbol::get (const std::string & name) const {
	return this-> getReferent ().get (name);
    }

    std::vector <Symbol> ISymbol::getLocal (const std::string &) const {
	return {};
    }    

    const Symbol& ISymbol::getReferent () const {
	if (this-> _referent == nullptr) return Symbol::__empty__;
	else return *this-> _referent;
    }
    
    Symbol& ISymbol::getReferent () {
	if (this-> _referent == nullptr) return Symbol::__empty__;
	else return *this-> _referent;
    }        

    void ISymbol::setReferent (Symbol * ref) {
	this-> _referent = ref;
    }

    std::string ISymbol::getRealName () const {
	if (this-> _referent == nullptr) return this-> _name.str;
	else {
	    auto ft = this-> _referent-> getRealName ();
	    if (ft != "")
		return ft + "::" + this-> _name.str;
	    else return this-> _name.str;
	}
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
	if (this-> _value != nullptr)
	    this-> _value-> insert (sym);
	else {
	    // We don't do anything, it is more convinient for global modules
	}
    }

    std::vector <Symbol> Symbol::get (const std::string & name) const {
	if (this-> _value == nullptr)
	    return {};
	return this-> _value-> get (name);
    }

    const Symbol & Symbol::getReferent () const {
	if (this-> _value == nullptr)
	    return Symbol::__empty__;
	return this-> _value-> getReferent ();
    }

    void Symbol::setReferent (Symbol * ref) {
	if (this-> _value != nullptr)
	    this-> _value-> setReferent (ref);
    }
    
    std::string Symbol::getRealName () const {
	if (this-> _value != nullptr) return this-> _value-> getRealName ();
	else {
	    return "";
	}
    }

    std::vector <Symbol> Symbol::getLocal (const std::string & name) const {
	if (this-> _value == nullptr) return {};
	else return this-> _value-> getLocal (name);
    }
    
    bool Symbol::equals (const Symbol & other) const {
	if (this-> _value == nullptr) return other._value == nullptr;
	return this-> _value-> equals (other);
    }


    
}
