#include <ymir/semantic/symbol/Enum.hh>

namespace semantic {

    Enum::Enum () :
	ISymbol (lexing::Word::eof (), false),
	_type (syntax::Expression::empty ()),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Enum::Enum (const lexing::Word & name, const std::vector <syntax::Expression> & values, const syntax::Expression & type, bool isWeak) :
	ISymbol (name, isWeak),
	_type (type),
	_fields (values),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol Enum::init (const lexing::Word & name, const std::vector <syntax::Expression> & values, const syntax::Expression & type, bool isWeak) {
	auto ret = Symbol {new (NO_GC) Enum (name, values, type, isWeak)};
	ret.to <Enum> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    bool Enum::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Enum thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    void Enum::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }
    
    void Enum::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    std::vector<Symbol> Enum::getTemplates () const {
	return this-> _table-> getTemplates ();
    }    
    
    void Enum::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    std::vector <Symbol> Enum::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table-> get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector <Symbol> Enum::getPublic (const std::string & name) const {
	auto vec = getReferent ().getPublic (name);
	auto local = this-> _table-> getPublic (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }
    
    std::vector <Symbol> Enum::getLocal (const std::string & name) const {
	return this-> _table-> get (name);
    }

    std::vector <Symbol> Enum::getLocalPublic (const std::string & name) const {
	return this-> _table-> getPublic (name);
    }    
    
    const syntax::Expression & Enum::getType () const {
	return this-> _type;
    }
    
    const std::vector <syntax::Expression> & Enum::getFields () const {
	return this-> _fields;
    }
    
    bool Enum::equals (const Symbol & other, bool parent) const {
	if (!other.is <Enum> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    std::string Enum::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
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
