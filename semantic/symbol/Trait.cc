#include <ymir/semantic/symbol/Trait.hh>
#include <ymir/semantic/symbol/Constructor.hh>
#include <ymir/syntax/expression/Var.hh>

namespace semantic {

    Trait::Trait () :
	ISymbol (lexing::Word::eof (), false), 
	_table (this),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Trait::Trait (const lexing::Word & name, bool isWeak) :
	ISymbol (name, isWeak),
	_table (this),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Trait::Trait (const Trait & other) :
	ISymbol (other),
	_table (other._table.clone (this)),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol Trait::init (const lexing::Word & name, bool isWeak) {
	return Symbol {new (Z0) Trait (name, isWeak)};
    }
    
    bool Trait::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Trait thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    void Trait::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }
    
    void Trait::insertTemplate (const Symbol & sym) {
	this-> _table.insertTemplate (sym);
    }

    std::vector<Symbol> Trait::getTemplates () const {
	return this-> _table.getTemplates ();
    }    
    
    void Trait::replace (const Symbol & sym) {
	this-> _table.replace (sym);
    }

    std::vector <Symbol> Trait::get (const std::string & name) const {
	return getReferent ().get (name);
    }

    std::vector <Symbol> Trait::getPublic (const std::string & name) const {
	return getReferent ().getPublic (name);
    }
    
    std::vector <Symbol> Trait::getLocal (const std::string &) const {
	return {};
    }

    bool Trait::equals (const Symbol & other, bool parent) const {
	if (!other.is <Trait> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const std::vector <Symbol> & Trait::getAllInner () const {
	return this-> _table.getAll ();
    }
    
    std::string Trait::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table.getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }

    const generator::Generator & Trait::getGenerator () const {
	return this-> _gen;
    }

    void Trait::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }

    void Trait::addField (const syntax::Expression & field) {
	this-> _fields.push_back (field);
    }

    const std::vector<syntax::Expression> & Trait::getFields () const {
	return this-> _fields;
    }
    
    void Trait::setPrivate (const std::string & name) {
	this-> _privates.push_back (name);
    }

    void Trait::setProtected (const std::string & name) {
	this-> _protected.push_back (name);
    }

    bool Trait::isMarkedPrivate (const std::string & name) const {
	for (auto & it : this-> _privates) {
	    if (it == name) return true;
	}
	return false;
    }

    bool Trait::isMarkedProtected (const std::string & name) const {
	for (auto & it : this-> _protected) {
	    if (it == name) return true;
	}
	return false;
    }

    std::string Trait::getRealName () const {
	return this-> getReferent ().getRealName ();
    }
    
}
