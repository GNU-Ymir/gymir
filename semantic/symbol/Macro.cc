#include <ymir/semantic/symbol/Macro.hh>

namespace semantic {

    Macro::Macro () :
	ISymbol (lexing::Word::eof (), "", false)
    {}

    Macro::Macro (const lexing::Word & name, const std::string & comments) :
	ISymbol (name, comments, false)
    {}

    Symbol Macro::init (const lexing::Word & name, const std::string & comments) {
	auto ret = Symbol {new (NO_GC) Macro (name, comments)};
	ret.to <Macro> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    bool Macro::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Macro thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    bool Macro::equals (const Symbol & other, bool parent) const {
	if (!other.is <Macro> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    void Macro::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }
    
    void Macro::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    std::vector<Symbol> Macro::getTemplates () const {
	return this-> _table-> getTemplates ();
    }    
    
    void Macro::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    std::vector <Symbol> Macro::get (const std::string & name) const {
	return getReferent ().get (name);
    }

    std::vector <Symbol> Macro::getPublic (const std::string & name) const {
	return getReferent ().getPublic (name);
    }
    
    std::vector <Symbol> Macro::getLocal (const std::string &) const {
	return {};
    }
    
    const std::vector <Symbol> & Macro::getAllInner () const {
	return this-> _table-> getAll ();
    }

    std::string Macro::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }    

}
