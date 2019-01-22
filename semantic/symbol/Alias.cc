#include <ymir/semantic/symbol/Alias.hh>

namespace semantic {

    Alias::Alias () :
	ISymbol (lexing::Word::eof ()),
	_value (syntax::Expression::empty ())
    {}

    Alias::Alias (const lexing::Word & name, const syntax::Expression & value) :
	ISymbol (name),
	_value (value)
    {}

    Symbol Alias::clone () const {
	return Alias::init (this-> getName (), this-> _value);
    }
    
    Symbol Alias::init (const lexing::Word & name, const syntax::Expression & value) {
	return Symbol {new (Z0) Alias (name, value)};
    }
    
    bool Alias::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Alias thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    bool Alias::equals (const Symbol & other) const {
	if (!other.is <Alias> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

    const syntax::Expression & Alias::getValue () const {
	return this-> _value;
    }   

}
