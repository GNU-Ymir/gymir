#include <ymir/semantic/symbol/Enum.hh>

namespace semantic {

    Enum::Enum () :
	ISymbol (lexing::Word::eof (), "", false),
	_type (syntax::Expression::empty ()),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Enum::Enum (const lexing::Word & name, const std::string & comments, const std::vector <syntax::Expression> & values, const syntax::Expression & type, const std::vector <std::string> & fieldComments, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_type (type),
	_fields (values),
	_gen (generator::Generator::empty ()),
	_field_comments (fieldComments)
    {}
    
    Symbol Enum::init (const lexing::Word & name, const std::string & comments, const std::vector <syntax::Expression> & values, const syntax::Expression & type, const std::vector <std::string> & fieldComments, bool isWeak) {
	auto ret = Symbol {new (NO_GC) Enum (name, comments, values, type,  fieldComments, isWeak)};
	ret.to <Enum> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    void Enum::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }
    
    void Enum::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    void Enum::getTemplates (std::vector<Symbol> & rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    
    
    void Enum::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    void Enum::get (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().get (name, rets);
	this-> _table-> get (name, rets);
    }

    void Enum::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
	this-> _table-> getPublic (name, rets);
    }
    
    void Enum::getLocal (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> get (name, rets);
    }

    void Enum::getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> getPublic (name, rets);
    }    
    
    const syntax::Expression & Enum::getType () const {
	return this-> _type;
    }
    
    const std::vector <syntax::Expression> & Enum::getFields () const {
	return this-> _fields;
    }

    const std::vector <std::string> & Enum::getFieldComments () const {
	return this-> _field_comments;
    }
    
    bool Enum::equals (const Symbol & other, bool parent) const {
	if (!other.is <Enum> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    std::string Enum::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }

    const generator::Generator & Enum::getGenerator () const {
	return this-> _gen;
    }

    void Enum::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }
    
}
