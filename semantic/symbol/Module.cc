#include <ymir/semantic/symbol/Module.hh>

namespace semantic {

    Module::Module () :
	ISymbol (lexing::Word::eof (), "", false)
    {}
    
    Module::Module (const lexing::Word & name, const std::string & comments, bool isWeak) :
	ISymbol (name, comments, isWeak)	
    {}
    
    Symbol Module::init (const lexing::Word & name, const std::string & comments, bool isWeak) {
	auto ret = Symbol {new (NO_GC) Module (name, comments, isWeak)};
	ret.to <Module> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    void Module::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }

    
    void Module::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    void Module::getTemplates (std::vector<Symbol> & rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    


    void Module::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    void Module::get (const std::string & name, std::vector <Symbol>& rets) const {
	getReferent ().get (name, rets);
	this-> _table-> get (name, rets);
    }

    void Module::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
	this-> _table-> getPublic (name, rets);
    }
        
    void Module::getLocal (const std::string & name, std::vector<Symbol> & rets) const {
	this-> _table-> get (name, rets);
    }

    void Module::getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> getPublic (name, rets);
    }

    const std::vector <Symbol> & Module::getAllLocal () const {
	return this-> _table-> getAll ();
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

    std::string Module::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }
    
}
