#include <ymir/semantic/symbol/Module.hh>

namespace semantic {

    Module::Module () :
	ISymbol (lexing::Word::eof ()),
	_table (ITable::init (this))
    {}
    
    Module::Module (const lexing::Word & name) :
	ISymbol (name),
	_table (ITable::init (this))
	
    {}

    Symbol Module::init (const lexing::Word & name) {
	return Symbol {new (Z0) Module (name)};
    }

    Symbol Module::clone () const {
	auto ret = new (Z0) Module (*this);
	ret-> _table.setAttach (ret);
	return Symbol {ret};
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

    std::vector <Symbol> Module::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector<Symbol> Module::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    const std::vector <Symbol> & Module::getAllLocal () const {
	return this-> _table.getAll ();
    }
    
    bool Module::equals (const Symbol & other) const {
	if (!other.is<Module> ()) return false;
	if (this-> getName () == other.getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else 
	    return false;
    }
    
    
}
