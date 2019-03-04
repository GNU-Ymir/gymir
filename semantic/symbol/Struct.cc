#include <ymir/semantic/symbol/Struct.hh>

namespace semantic {


    Struct::Struct () :
	ISymbol (lexing::Word::eof ()),
	_isPacked (false),
	_isUnion (false),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Struct::Struct (const lexing::Word & name, const std::vector <syntax::Expression> & fields) :
	ISymbol (name),
	_isPacked (false),
	_isUnion (false),
	_fields (fields),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol Struct::init (const lexing::Word & name, const std::vector <syntax::Expression> & fields) {
	return Symbol {new (Z0) Struct (name, fields)};
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

    bool Struct::equals (const Symbol & other) const {
	if (!other.is <Struct> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

    const std::vector <syntax::Expression> & Struct::getFields () const {
	return this-> _fields;
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
	return buf.str ();
    }

    const generator::Generator & Struct::getGenerator () const {
	return this-> _gen;
    }

    void Struct::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }
    
}
