#include <ymir/semantic/symbol/Module.hh>

namespace semantic {

    Module::Module () :
	ISymbol (lexing::Word::eof ()),
	_table (this)
    {}
    
    Module::Module (const lexing::Word & name) :
	ISymbol (name),
	_table (this)	
    {}

    Module::Module (const Module & mod) :
	ISymbol (mod),
	_table (mod._table.clone (this))
    {}

    Symbol Module::init (const lexing::Word & name) {
	return Symbol {new (Z0) Module (name)};
    }

    Symbol Module::clone () const {
	return Symbol {new Module (*this)};
    }

    bool Module::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Module thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);
    }

    void Module::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }

    
    void Module::insertTemplate (const Symbol & sym) {
	this-> _table.insertTemplate (sym);
    }

    std::vector<Symbol> Module::getTemplates () const {
	return this-> _table.getTemplates ();
    }    


    void Module::replace (const Symbol & sym) {
	this-> _table.replace (sym);
    }

    std::vector <Symbol> Module::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector <Symbol> Module::getPublic (const std::string & name) const {
	auto vec = getReferent ().getPublic (name);
	auto local = this-> _table.getPublic (name);
	
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }
    
    std::vector<Symbol> Module::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    std::vector<Symbol> Module::getLocalPublic (const std::string & name) const {
	return this-> _table.getPublic (name);
    }

    const std::vector <Symbol> & Module::getAllLocal () const {
	return this-> _table.getAll ();
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
	for (auto & it : this-> _table.getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }
    
}
