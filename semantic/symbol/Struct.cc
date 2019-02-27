#include <ymir/semantic/symbol/Struct.hh>

namespace semantic {


    Struct::Struct () :
	ISymbol (lexing::Word::eof ()),
	_table (this)
    {}

    Struct::Struct (const lexing::Word & name) :
	ISymbol (name),
	_table (this)
    {}

    Struct::Struct (const Struct & ot) :
	ISymbol (ot),
	_table (ot._table.clone (this))
    {}
    
    Symbol Struct::init (const lexing::Word & name) {
	return Symbol {new (Z0) Struct (name)};
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

    void Struct::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }
    
    void Struct::replace (const Symbol & sym) {
	this-> _table.replace (sym);
    }
    
    std::vector <Symbol> Struct::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector <Symbol> Struct::getLocal (const std::string & name) const {
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

    std::string Struct::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table.getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }
}
