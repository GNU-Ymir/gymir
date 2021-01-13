#include <ymir/semantic/symbol/MacroConstructor.hh>

namespace semantic {

    MacroConstructor::MacroConstructor () :
	ISymbol (lexing::Word::eof (), "", false),
	_constr (syntax::Declaration::empty ())
    {}
    
    MacroConstructor::MacroConstructor (const lexing::Word & name, const std::string & comments, const syntax::Declaration & constr) :
	ISymbol (name, comments, false),
	_constr (constr)
    {}

    Symbol MacroConstructor::init (const lexing::Word & name, const std::string & comments, const syntax::Declaration & constr) {
	auto ret = Symbol {new (NO_GC) MacroConstructor (name, comments, constr)};
	ret.to <MacroConstructor> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    void MacroConstructor::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }

    void MacroConstructor::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }
    
    void MacroConstructor::getTemplates (std::vector <Symbol> & rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    
    
    void MacroConstructor::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    void MacroConstructor::get (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().get (name, rets);
	this-> _table-> get (name, rets);
    }

    void MacroConstructor::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
	this-> _table-> getPublic (name, rets);
    }
    
    void MacroConstructor::getLocal (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> get (name, rets);
    }

    void MacroConstructor::getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> getPublic (name, rets);
    }
    
    bool MacroConstructor::equals (const Symbol & other, bool parent) const {
	if (!other.is <MacroConstructor> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const syntax::Declaration & MacroConstructor::getContent () const {
	return this-> _constr;
    }
    
    std::string MacroConstructor::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }    

}
