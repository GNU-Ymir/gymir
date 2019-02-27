#include <ymir/semantic/symbol/ModRef.hh>
#include <algorithm>

namespace semantic {

    ModRef::ModRef () :
	ISymbol (lexing::Word::eof ()),
	_name (""),
	_table (this)
    {}
    
    ModRef::ModRef (const lexing::Word & loc, const std::string & name) :
	ISymbol ({loc, name}),
	_name (name),
	_table (this)
    {}


    ModRef::ModRef (const ModRef & mod) :
	ISymbol (mod),
	_name (mod._name),
	_table (mod._table.clone (this))
    {}
    
    Symbol ModRef::init (const lexing::Word & loc, const std::string & name) {
	return Symbol {new (Z0) ModRef (loc, name)};
    }

    Symbol ModRef::init (const lexing::Word & loc, const std::vector <std::string> & names_) {
	auto names = names_;      
	std::reverse (names.begin (), names.end ());

	Symbol current = Symbol::empty ();
	for (auto & name : names) {
	    auto back = ModRef::init (loc, name);
	    if (!current.isEmpty ()) 
		back.insert (current);	    
	    current = back;
	}
	
	return current;
    }
    
    Symbol ModRef::clone () const {
	return Symbol {new ModRef (*this)};
    }

    bool ModRef::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	ModRef thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);
    }

    void ModRef::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }   
    
    bool ModRef::equals (const Symbol & other) const {
	if (!other.is<ModRef> ()) return false;
	return this-> getName () == other.getName () && this-> _name == other.to<ModRef> ()._name;
    }

    Symbol ModRef::merge (const ModRef & other) const {
	std::vector <Symbol> ret;
	for (auto & i_ : this-> _table.getAll ()) {
	    auto & i = i_.to<ModRef> ();
	    bool add = true;
	    for (auto & j_ : other._table.getAll ()) {
		auto & j = j_.to<ModRef> ();
		if (j._name == i._name && j._table.getAll ().size () != 0 && i._table.getAll ().size () != 0) {
		    add = false;
		    ret.push_back (i.merge (j));
		    break;
		}
	    }
	    if (add) ret.push_back (i_);
	}

	for (auto & j_ : other._table.getAll ()) {
	    auto & j = j_.to<ModRef> ();
	    bool add = true;
	    for (auto & z_ : ret) {
		auto & z = z_.to<ModRef> ();
		if (j._name == z._name && j._table.getAll ().size () != 0 && z._table.getAll ().size () != 0) {
		    add = false;
		    break;
		}
	    }
	    if (add) ret.push_back (j_);
	}

	auto mod = ModRef::init (this-> getName (), this-> _name);
	for (auto sym : ret) {
	    mod.insert (sym);
	}
	return mod;
    }
    
    const std::string & ModRef::getModName () const {
	return this-> _name;
    }

    std::vector <Symbol> ModRef::getLocal (const std::string & name) const {
	// This is a leaf, we have the right to access to the data of this module	
	if (this-> _table.getAll ().size () == 0) {
	    auto real_name = this-> getRealName ();
	    auto  mod = Symbol::getModuleByPath (real_name);
	    return mod.getLocal (name);
	} else {
	    return this-> _table.get (name);
	}	
    }

    std::string ModRef::getRealName () const {
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
	buf.writefln ("%*- %", i, "|\t", this-> getModName ());
	for (auto inner : this-> _table.getAll ())
	    buf.write (inner.formatTree (i + 1));
	return buf.str ();
    }
}