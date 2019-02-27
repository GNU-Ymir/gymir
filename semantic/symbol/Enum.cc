#include <ymir/semantic/symbol/Enum.hh>

namespace semantic {


    Enum::Enum () :
	ISymbol (lexing::Word::eof ()),
	_table (ITable::init (this)),
	_type (syntax::Expression::empty ())
    {}

    Enum::Enum (const lexing::Word & name, const syntax::Expression & type) :
	ISymbol (name),
	_table (ITable::init (this)),
	_type (type)
    {}

    Symbol Enum::init (const lexing::Word & name, const syntax::Expression & type) {
	return Symbol {new (Z0) Enum (name, type)};
    }

    Symbol Enum::clone () const {
	return Symbol {new (Z0) Enum (*this)};
    }

    bool Enum::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Enum thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    void Enum::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }

    std::vector <Symbol> Enum::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector <Symbol> Enum::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    bool Enum::equals (const Symbol & other) const {
	if (!other.is <Enum> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

}
