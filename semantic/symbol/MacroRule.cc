#include <ymir/semantic/symbol/MacroRule.hh>

namespace semantic {

    MacroRule::MacroRule () :
	ISymbol (lexing::Word::eof (), "", false),
	_constr (syntax::Declaration::empty ())
    {}
    
    MacroRule::MacroRule (const lexing::Word & name, const std::string & comments, const syntax::Declaration & constr) :
	ISymbol (name, comments, false),
	_constr (constr)
    {}

    Symbol MacroRule::init (const lexing::Word & name, const std::string & comments, const syntax::Declaration & constr) {
	auto ret = Symbol {new (NO_GC) MacroRule (name, comments, constr)};
	ret.to <MacroRule> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    
    void MacroRule::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }

    void MacroRule::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }
    
    void MacroRule::getTemplates (std::vector <Symbol> & rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    
    
    // void MacroRule::replace (const Symbol & sym) {
    // 	this-> _table-> replace (sym);
    // }

    void MacroRule::get (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().get (name, rets);
	this-> _table-> get (name, rets);
    }

    void MacroRule::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
	this-> _table-> getPublic (name, rets);
    }
    
    void MacroRule::getLocal (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> get (name, rets);
    }

    void MacroRule::getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> getPublic (name, rets);
    }


    bool MacroRule::equals (const Symbol & other, bool parent) const {
	if (!other.is <MacroRule> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const syntax::Declaration & MacroRule::getContent () const {
	return this-> _constr;
    }
    
    std::string MacroRule::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }    

}
