#include <ymir/semantic/symbol/Trait.hh>
#include <ymir/semantic/symbol/Constructor.hh>
#include <ymir/syntax/expression/Var.hh>

namespace semantic {

    Trait::Trait () :
	ISymbol (lexing::Word::eof (), "", false), 
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Trait::Trait (const lexing::Word & name, const std::string & comments, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol Trait::init (const lexing::Word & name, const std::string & comments, bool isWeak) {
	auto ret = Symbol {new (NO_GC) Trait (name, comments, isWeak)};
	ret.to <Trait> ()._table = Table::init (ret.getPtr ());
	return ret;
    }
    
    void Trait::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }
    
    void Trait::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    void Trait::getTemplates (std::vector <Symbol> & rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    
    
    void Trait::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    void Trait::get (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().get (name, rets);
    }

    void Trait::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
    }
    
    void Trait::getLocal (const std::string &, std::vector <Symbol> &) const {}

    bool Trait::equals (const Symbol & other, bool parent) const {
	if (!other.is <Trait> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const std::vector <Symbol> & Trait::getAllInner () const {
	return this-> _table-> getAll ();
    }
    
    std::string Trait::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }

    const generator::Generator & Trait::getGenerator () const {
	return this-> _gen;
    }

    void Trait::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }

    void Trait::addField (const syntax::Expression & field) {
	this-> _fields.push_back (field);
    }

    const std::vector<syntax::Expression> & Trait::getFields () const {
	return this-> _fields;
    }
    
    void Trait::setPrivate (const std::string & name) {
	this-> _privates.push_back (name);
    }

    void Trait::setProtected (const std::string & name) {
	this-> _protected.push_back (name);
    }

    bool Trait::isMarkedPrivate (const std::string & name) const {
	for (auto & it : this-> _privates) {
	    if (it == name) return true;
	}
	return false;
    }

    bool Trait::isMarkedProtected (const std::string & name) const {
	for (auto & it : this-> _protected) {
	    if (it == name) return true;
	}
	return false;
    }

    std::string Trait::getRealName () const {
	return this-> getReferent ().getRealName ();
    }
    
}
