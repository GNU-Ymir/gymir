#include <ymir/semantic/symbol/Enum.hh>

namespace semantic {


    Enum::Enum () :
	ISymbol (lexing::Word::eof ()),
	_table (this),
	_type (syntax::Expression::empty ()),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Enum::Enum (const lexing::Word & name, const std::vector <syntax::Expression> & values, const syntax::Expression & type) :
	ISymbol (name),
	_table (this),
	_type (type),
	_fields (values),
	_gen (generator::Generator::empty ())
    {}

    Enum::Enum (const Enum & other) :
	ISymbol (other),
	_table (other._table.clone (this)),
	_type (other._type),
	_fields (other._fields),
	_gen (other._gen)
    {}
    
    Symbol Enum::init (const lexing::Word & name, const std::vector <syntax::Expression> & values, const syntax::Expression & type) {
	return Symbol {new (Z0) Enum (name, values, type)};
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
    
    void Enum::insertTemplate (const Symbol & sym) {
	this-> _table.insertTemplate (sym);
    }

    std::vector<Symbol> Enum::getTemplates () const {
	return this-> _table.getTemplates ();
    }    
    
    void Enum::replace (const Symbol & sym) {
	this-> _table.replace (sym);
    }

    std::vector <Symbol> Enum::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector <Symbol> Enum::getPublic (const std::string & name) const {
	auto vec = getReferent ().getPublic (name);
	auto local = this-> _table.getPublic (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }
    
    std::vector <Symbol> Enum::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    std::vector <Symbol> Enum::getLocalPublic (const std::string & name) const {
	return this-> _table.getPublic (name);
    }    
    
    const syntax::Expression & Enum::getType () const {
	return this-> _type;
    }
    
    const std::vector <syntax::Expression> & Enum::getFields () const {
	return this-> _fields;
    }
    
    bool Enum::equals (const Symbol & other) const {
	if (!other.is <Enum> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

    std::string Enum::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table.getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }

    const generator::Generator & Enum::getGenerator () const {
	return this-> _gen;
    }

    void Enum::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }
    
}
