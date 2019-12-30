#include <ymir/semantic/symbol/Class.hh>
#include <ymir/semantic/symbol/Constructor.hh>
#include <ymir/syntax/expression/Var.hh>

namespace semantic {

    Class::Class () :
	ISymbol (lexing::Word::eof ()), 
	_table (this),
	_ancestor (syntax::Expression::empty ()),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Class::Class (const lexing::Word & name, const syntax::Expression & ancestor) :
	ISymbol (name),
	_table (this),
	_ancestor (ancestor),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}

    Class::Class (const Class & other) :
	ISymbol (other),
	_table (other._table.clone (this)),
	_ancestor (other._ancestor),
	_fields ({}),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol Class::init (const lexing::Word & name, const syntax::Expression & ancestor) {
	return Symbol {new (Z0) Class (name, ancestor)};
    }

    Symbol Class::clone () const {
	return Symbol {new Class (*this)};
    }
    
    bool Class::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Class thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    void Class::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }
    
    void Class::insertTemplate (const Symbol & sym) {
	this-> _table.insertTemplate (sym);
    }

    std::vector<Symbol> Class::getTemplates () const {
	return this-> _table.getTemplates ();
    }    
    
    void Class::replace (const Symbol & sym) {
	this-> _table.replace (sym);
    }

    std::vector <Symbol> Class::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector <Symbol> Class::getPublic (const std::string & name) const {
	auto vec = getReferent ().getPublic (name);
	auto local = this-> _table.getPublic (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }
    
    std::vector <Symbol> Class::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    bool Class::equals (const Symbol & other) const {
	if (!other.is <Class> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

    const std::vector <Symbol> & Class::getAllLocal () const {
	return this-> _table.getAll ();
    }
    
    std::string Class::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table.getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }

    const generator::Generator & Class::getGenerator () const {
	return this-> _gen;
    }

    void Class::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }

    void Class::addField (const syntax::Expression & field) {
	this-> _fields.push_back (field);
    }

    const std::vector<syntax::Expression> & Class::getFields () const {
	return this-> _fields;
    }
    
    const syntax::Expression & Class::getAncestor () const {
	return this-> _ancestor;
    }
    
}
