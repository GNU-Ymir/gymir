#include <ymir/semantic/symbol/Impl.hh>

namespace semantic {

    Impl::Impl () :
	ISymbol (lexing::Word::eof (), "", false),
	_trait (syntax::Expression::empty ())
    {}
    
    Impl::Impl (const lexing::Word & name, const std::string & comments, const syntax::Expression & trait, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_trait (trait)
    {}

    Symbol Impl::init (const lexing::Word & name, const std::string & comments, const syntax::Expression & trait, bool isWeak) {
	auto ret = Symbol {new (NO_GC) Impl (name, comments, trait, isWeak)};
	ret.to <Impl> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    void Impl::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }
   

    void Impl::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    void Impl::get (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().get (name, rets);
	this-> _table-> get (name, rets);
    }

    void Impl::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
	this-> _table-> getPublic (name, rets);
    }
    
    void Impl::getLocal (const std::string & name, std::vector<Symbol> & rets) const {
	this-> _table-> get (name, rets);
    }

    void Impl::getLocalPublic (const std::string & name, std::vector<Symbol> & rets) const {
	this-> _table-> getPublic (name, rets);
    }

    const std::vector <Symbol> & Impl::getAllInner () const {
	return this-> _table-> getAll ();
    }
    
    bool Impl::equals (const Symbol & other, bool parent) const {
	if (!other.is<Impl> ()) return false;
	if (this-> getName () == other.getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else 
	    return false;
    }

    std::string Impl::getRealName () const {
	return this-> getReferent ().getRealName ();
    }
    
    const syntax::Expression & Impl::getTrait () const {
	return this-> _trait;
    }    
    
    std::string Impl::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }
    
}
