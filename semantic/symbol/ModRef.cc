#include <ymir/semantic/symbol/ModRef.hh>
#include <algorithm>

namespace semantic {
    
    ModRef::ModRef (const lexing::Word & loc, const std::string & comments, const std::string & name, bool isWeak, bool isTrusted) :
	ISymbol (lexing::Word::init (loc, name), comments, isWeak),
	_name (name),
	_module (this, &ModRef::findModule),
	_emptyModule (this, &ModRef::emptyModule)
    {
	if (isTrusted)
	    this-> setTrusted ();
	this-> setPublic ();
    }
    
    Symbol ModRef::init (const lexing::Word & loc, const std::string & comments, const std::string & name, bool isWeak, bool isTrusted) {
	auto ret = Symbol {new (NO_GC) ModRef (loc, comments, name, isWeak, isTrusted)};
	ret.to <ModRef> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    Symbol ModRef::init (const lexing::Word & loc, const std::string & comments, const std::vector <std::string> & names_, bool isWeak, bool isTrusted) {
	auto names = names_;      
	std::reverse (names.begin (), names.end ());

	Symbol current = Symbol::empty ();
	for (auto & name : names) {
	    auto back = ModRef::init (loc, comments, name, isWeak, isTrusted);
	    if (!current.isEmpty ()) 
		back.insert (current);	    
	    current = back;
	}
	
	return current;
    }
    
    void ModRef::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }   

    void ModRef::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    void ModRef::insertOrReplace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }
    
    void ModRef::getTemplates (std::vector <Symbol> & rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    
    
    bool ModRef::equals (const Symbol & other, bool) const {
	if (!other.is<ModRef> ()) return false;
	return this-> getName () == other.getName () && this-> _name == other.to<ModRef> ()._name;
    }

    Symbol ModRef::merge (const ModRef & other) const {
	std::vector <Symbol> ret;
	for (auto & i_ : this-> _table-> getAll ()) {
	    auto & i = i_.to<ModRef> ();
	    bool add = true;
	    for (auto & j_ : other._table-> getAll ()) {
		auto & j = j_.to<ModRef> ();		
		if (j._name == i._name) {
		    if ((j._table-> getAll ().size () != 0 && i._table-> getAll ().size () != 0) ||
			(j._table-> getAll ().size () == 0 && i._table-> getAll ().size () == 0) 
		    ) {
			add = false;
			ret.push_back (i.merge (j));
			break;
		    } 
		}
	    }
	    if (add) ret.push_back (i_);
	}

	for (auto & j_ : other._table-> getAll ()) {
	    auto & j = j_.to<ModRef> ();
	    bool add = true;
	    for (auto & z_ : ret) {
		auto & z = z_.to<ModRef> ();
		if (j._name == z._name) {
		    if ((j._table-> getAll ().size () != 0 && z._table-> getAll ().size () != 0) ||
			(j._table-> getAll ().size () == 0 && z._table-> getAll ().size () == 0)
		    ) {
			add = false;
			break;
		    }
		}
	    }
	    if (add) ret.push_back (j_);
	}

	auto mod = ModRef::init (this-> getName (), this-> getComments (), this-> _name, this-> isWeak () && other.isWeak (), other.isTrusted () && this-> isTrusted ());
	for (auto sym : ret) {
	    mod.insert (sym);
	}
	return mod;
    }
    
    const std::string & ModRef::getModName () const {
	return this-> _name;
    }

    void ModRef::getLocal (const std::string & name, std::vector <Symbol> & rets) const {
	// This is a leaf, we have the right to access to the data of this module
	
	this-> _table-> get (name, rets);
	this-> _module.getValue ().getLocal (name, rets);
	
	Symbol::mergeEqSymbols (rets);
    }

    void ModRef::getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const {
	// This is a leaf, we have the right to access to the data of this module
	this-> _table-> getPublic (name, rets);
	this-> _module.getValue ().getLocalPublic (name, rets);
	
	Symbol::mergeEqSymbols (rets);
    }

    const Ymir::Lazy<Symbol, ModRef> & ModRef::getModule () const {
	if (this-> _table-> getAll ().size () == 0) {
	    return this-> _module;
	} else return this-> _emptyModule;
    }

    Symbol ModRef::findModule () const {
	auto real_name = this-> getRealName ().getValue ();
	auto  mod = Symbol::getModuleByPath (real_name);
	return mod;	    
    }

    Symbol ModRef::emptyModule () const {
	return Symbol::empty ();
    }
    
    std::string ModRef::computeRealName () const {
	if (this-> getReferent ().isEmpty ()) return this-> getModName ();
	else if (!this-> getReferent ().is <ModRef> ()) return this-> getModName ();
	else {
	    auto ft = this-> getReferent ().getRealName ();
	    if (ft != "")
		return ft + "::" + this-> getModName ();
	    else return this-> getModName ();
	}
    }
    
    std::string ModRef::formatTree (int i) const {
	Ymir::OutBuffer buf;	
	buf.writefln ("%*- %", i, "|\t", this-> getRealName ().getValue ());
	for (auto inner : this-> _table-> getAll ())
	    buf.write (inner.formatTree (i + 1));
	return buf.str ();
    }

    void ModRef::setReferent (const Symbol & sym) {
	ISymbol::setReferent (sym);
	this-> _module.unvalidate ();
    }
}
