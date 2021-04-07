#include <ymir/semantic/symbol/Module.hh>
#include <ymir/semantic/symbol/ModRef.hh>

namespace semantic {

    Module::Module () :
	ISymbol (lexing::Word::eof (), "", false),
	_isExtern (false),
	_modRef (Symbol::empty ())
    {}
    
    Module::Module (const lexing::Word & name, const std::string & comments, bool isWeak, bool isTrusted, bool isGlobal) :
	ISymbol (name, comments, isWeak),
	_isExtern (false),
	_modRef (ModRef::init (name, "", name.getStr (), isWeak, isTrusted)),
	_isGlobal (isGlobal)
    {
	if (isTrusted)
	    this-> setTrusted ();
    }
    
    Symbol Module::init (const lexing::Word & name, const std::string & comments, bool isWeak, bool isTrusted, bool isGlobal) {
	auto ret = Symbol {new (NO_GC) Module (name, comments, isWeak, isTrusted, isGlobal)};
	ret.to <Module> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    void Module::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
	if (sym.is<Module> ()) {
	    this-> _modRef.insertOrReplace (sym.to <Module> ().getModRef ());
	}
    }

    void Module::insertOrReplace (const Symbol & sym) {
	this-> _table-> replace (sym);
	if (sym.is<Module> ()) {
	    this-> _modRef.insertOrReplace (sym.to <Module> ().getModRef ());
	}
    }
    
    void Module::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    void Module::getTemplates (std::vector<Symbol> & rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    


    // void Module::replace (const Symbol & sym) {
    // 	this-> _table-> replace (sym);
    // }

    void Module::get (const std::string & name, std::vector <Symbol>& rets) const {
	getReferent ().get (name, rets);
	this-> _table-> get (name, rets);
	if (// this-> _isGlobal && 
	    name == this-> getName ().getStr ()) {
	    rets.push_back (this-> _modRef);
	}
    }

    void Module::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
	this-> _table-> getPublic (name, rets);
	if (// this-> _isGlobal &&
	    name == this-> getName ().getStr ()) {
	    rets.push_back (this-> _modRef);
	}
    }
        
    void Module::getLocal (const std::string & name, std::vector<Symbol> & rets) const {
	this-> _table-> get (name, rets);
	if (// this-> _isGlobal && 
	    name == this-> getName ().getStr ()) {
	    rets.push_back (this-> _modRef);
	}
    }

    void Module::getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> getPublic (name, rets);
	if (// this-> _isGlobal && 
	    name == this-> getName ().getStr ()) {
	    rets.push_back (this-> _modRef);
	}
    }

    const std::vector <Symbol> & Module::getAllLocal () const {
	return this-> _table-> getAll ();
    }

    bool Module::isGlobal () const {
	return this-> _isGlobal;
    }
    
    bool Module::equals (const Symbol & other, bool parent) const {
	if (!other.is<Module> ()) return false;
	if (this-> getName () == other.getName ()) {
	    if (parent) {		
		return this-> getReferent ().equals (other.getReferent ());
	    } else return true;
	} else 
	    return false;
    }
    
    bool Module::isExtern () const {
	return this-> _isExtern;
    }

    void Module::isExtern (bool is) {
	this-> _isExtern = is;
    }

    const Symbol & Module::getModRef () const {
	return this-> _modRef;
    }
    
    std::string Module::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	
	buf.writefln ("%*- %", i, "|\t", this-> _modRef.formatTree ());
	return buf.str ();
    }
    
}
