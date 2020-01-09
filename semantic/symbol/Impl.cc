#include <ymir/semantic/symbol/Impl.hh>

namespace semantic {

    Impl::Impl () :
	ISymbol (lexing::Word::eof ()),
	_table (this),
	_trait (syntax::Expression::empty ())
    {}
    
    Impl::Impl (const lexing::Word & name, const syntax::Expression & trait) :
	ISymbol (name),
	_table (this),
	_trait (trait)
    {}

    Impl::Impl (const Impl & mod) :
	ISymbol (mod),
	_table (mod._table.clone (this)),
	_trait (mod._trait)
    {}

    Symbol Impl::init (const lexing::Word & name, const syntax::Expression & trait) {
	return Symbol {new (Z0) Impl (name, trait)};
    }

    Symbol Impl::clone () const {
	return Symbol {new Impl (*this)};
    }

    bool Impl::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Impl thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);
    }

    void Impl::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }
   

    void Impl::replace (const Symbol & sym) {
	this-> _table.replace (sym);
    }

    std::vector <Symbol> Impl::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector <Symbol> Impl::getPublic (const std::string & name) const {
	auto vec = getReferent ().getPublic (name);
	auto local = this-> _table.getPublic (name);
	
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }
    
    std::vector<Symbol> Impl::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    std::vector<Symbol> Impl::getLocalPublic (const std::string & name) const {
	return this-> _table.getPublic (name);
    }

    const std::vector <Symbol> & Impl::getAllInner () const {
	return this-> _table.getAll ();
    }
    
    bool Impl::equals (const Symbol & other) const {
	if (!other.is<Impl> ()) return false;
	if (this-> getName () == other.getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else 
	    return false;
    }

    const syntax::Expression & Impl::getTrait () const {
	return this-> _trait;
    }    

    std::string Impl::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table.getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }
    
}
