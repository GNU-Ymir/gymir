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

    void Macro::getTemplates (std::vector<Symbol>& rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    
    
    // void Macro::replace (const Symbol & sym) {
    // 	this-> _table-> replace (sym);
    // }

    void Macro::get (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().get (name, rets);
    }

    void Macro::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
    }
    
    void Macro::getLocal (const std::string &, std::vector <Symbol> &) const {
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
