#include <ymir/semantic/symbol/Enum.hh>

namespace semantic {


    Enum::Enum () :
	ISymbol (lexing::Word::eof ()),
	_table (ITable::init (this)),
	_overload (),
	_type (syntax::Expression::empty ())
    {}

    Enum::Enum (const lexing::Word & name, const syntax::Expression & type) :
	ISymbol (name),
	_table (ITable::init (this)),
	_overload (),
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

    const std::vector <Symbol> & Enum::getOverloading () const {
	return this-> _overload;
    }

    void Enum::setOverloading (const std::vector <Symbol> & overs) {
	this-> _overload = overs;
    }

    void Enum::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }

    std::vector <Symbol> Enum::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	const Symbol & local = this-> _table.get (name);
	if (!local.isEmpty ())
	    vec.push_back (local);
	return vec;
    }

    const Symbol & Enum::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    bool Enum::equals (const Symbol & other) const {
	if (!other.is <Enum> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

}
