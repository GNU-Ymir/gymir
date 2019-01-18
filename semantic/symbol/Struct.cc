#include <ymir/semantic/symbol/Struct.hh>

namespace semantic {


    Struct::Struct () :
	ISymbol (lexing::Word::eof ()),
	_table (ITable::init (this)),
	_overload (),
	_content (syntax::Declaration::empty ())
    {}

    Struct::Struct (const lexing::Word & name, const syntax::Struct & str) :
	ISymbol (name),
	_table (ITable::init (this)),
	_overload (),
	_content (syntax::Declaration {str.clone ()})
    {}

    Symbol Struct::init (const lexing::Word & name, const syntax::Struct & str) {
	return Symbol {new (Z0) Struct (name, str)};
    }

    Symbol Struct::clone () const {
	return Symbol {new (Z0) Struct (*this)};
    }

    bool Struct::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Struct thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    const std::vector <Symbol> & Struct::getOverloading () const {
	return this-> _overload;
    }

    void Struct::setOverloading (const std::vector <Symbol> & overs) {
	this-> _overload = overs;
    }

    void Struct::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }

    std::vector <Symbol> Struct::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	const Symbol & local = this-> _table.get (name);
	if (!local.isEmpty ())
	    vec.push_back (local);
	return vec;
    }

    const Symbol & Struct::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    bool Struct::equals (const Symbol & other) const {
	if (!other.is <Struct> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

    bool Struct::isUnion () const {
	return this-> _isUnion;
    }

    void Struct::isUnion (bool is) {
	this-> _isUnion = is;
    }

    bool Struct::isPacked () const {
	return this-> _isPacked;
    }

    void Struct::isPacked (bool is) {
	this-> _isPacked = is;
    }

}
