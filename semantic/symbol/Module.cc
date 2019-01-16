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

    const Symbol & Module::get (const std::string & name) const {
	const Symbol & local = this-> _table.get (name);
	if (local.isEmpty ()) return getReferent ().get (name);
	else return local;
    }

    
}
