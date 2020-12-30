#include <ymir/semantic/symbol/Constructor.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace semantic {

    Constructor::Constructor () :
	ISymbol (lexing::Word::eof (), "", false),
	_content (syntax::Declaration::empty ())
    {}

    Constructor::Constructor (const lexing::Word & name, const std::string & comments, const syntax::Constructor & func, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_content (syntax::Constructor::init (func))
    {}

    Symbol Constructor::init (const lexing::Word & name, const std::string & comments, const syntax::Constructor & func, bool isWeak) {
	auto ret = Symbol {new (NO_GC) Constructor (name, comments, func, isWeak)};
	ret.to <Constructor> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    bool Constructor::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Constructor thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);
    }

    void Constructor::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }

    void Constructor::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }
    
    std::vector<Symbol> Constructor::getTemplates () const {
	return this-> _table-> getTemplates ();
    }    
    
    void Constructor::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    std::vector<Symbol> Constructor::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table-> get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector<Symbol> Constructor::getPublic (const std::string & name) const {
	auto vec = getReferent ().getPublic (name);
	auto local = this-> _table-> getPublic (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }
    
    std::vector <Symbol> Constructor::getLocal (const std::string & name) const {
	return this-> _table-> get (name);
    }

    std::vector <Symbol> Constructor::getLocalPublic (const std::string & name) const {
	return this-> _table-> getPublic (name);
    }
    
    bool Constructor::equals (const Symbol & other, bool parent) const {
	if (!other.is <Constructor> ()) return false;
	if (other.getName ().isSame (this-> getName ())) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else
	    return false;
    }

    const syntax::Constructor & Constructor::getContent () const {
	return this-> _content.to <syntax::Constructor> ();
    }

    std::string Constructor::getRealName () const {
	if (this-> getName () == Keys::MAIN) return this-> getName ().getStr ();
	else return ISymbol::getRealName ();
    }

    std::string Constructor::getMangledName () const {
	if (this-> getName () == Keys::MAIN) return this-> getName ().getStr ();
	else return ISymbol::getMangledName ();
    }

    std::string Constructor::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }

    void Constructor::setClass (const Symbol& sym) {
	this-> _class = sym.getPtr ();
    }
     
    Symbol Constructor::getClass () const {
	return Symbol {this-> _class};
    }

    void Constructor::setThrowers (const std::vector <syntax::Expression> & throwers) {
	this-> _throwers = throwers;
    }

    const std::vector <syntax::Expression> & Constructor::getThrowers () const {
	return this-> _throwers;
    }

    const std::vector <lexing::Word> & Constructor::getCustomAttributes () const {
	return this-> _content.to <syntax::Constructor> ().getCustomAttributes ();
    }
    
}
