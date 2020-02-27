#include <ymir/semantic/symbol/Class.hh>
#include <ymir/semantic/symbol/Constructor.hh>
#include <ymir/syntax/expression/Var.hh>

namespace semantic {

    Class::Class () :
	ISymbol (lexing::Word::eof (), false), 
	_ancestor (syntax::Expression::empty ()),
	_fields ({}),
	_gen (generator::Generator::empty ()),
	_typeInfo (generator::Generator::empty ())
    {}

    Class::Class (const lexing::Word & name, const syntax::Expression & ancestor, bool isWeak) :
	ISymbol (name, isWeak),
	_ancestor (ancestor),
	_fields ({}),
	_gen (generator::Generator::empty ()),
	_typeInfo (generator::Generator::empty ())
    {}

    
    Symbol Class::init (const lexing::Word & name, const syntax::Expression & ancestor, bool isWeak) {
	auto ret = Symbol {new (Z0) Class (name, ancestor, isWeak)};
	ret.to <Class> ()._table = Table::init (ret.getPtr ());
	return ret;
    }
    
    bool Class::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Class thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }
    
    void Class::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }
    
    void Class::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    std::vector<Symbol> Class::getTemplates () const {
	return this-> _table-> getTemplates ();
    }    
    
    void Class::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    std::vector <Symbol> Class::get (const std::string & name) const {
	return getReferent ().get (name);
    }

    std::vector <Symbol> Class::getPublic (const std::string & name) const {
	return getReferent ().getPublic (name);
    }
    
    std::vector <Symbol> Class::getLocal (const std::string &) const {
	return {};
    }

    bool Class::equals (const Symbol & other, bool parent) const {
	if (!other.is <Class> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const std::vector <Symbol> & Class::getAllInner () const {
	return this-> _table-> getAll ();
    }
    
    std::string Class::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table-> getAll ()) {
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

    const generator::Generator & Class::getTypeInfo () const {
	return this-> _typeInfo;
    }

    void Class::setTypeInfo (const generator::Generator & gen) {
	this-> _typeInfo = gen;
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

    void Class::setPrivate (const std::string & name) {
	this-> _privates.push_back (name);
    }

    void Class::setProtected (const std::string & name) {
	this-> _protected.push_back (name);
    }

    bool Class::isMarkedPrivate (const std::string & name) const {
	for (auto & it : this-> _privates) {
	    if (it == name) return true;
	}
	return false;
    }
    
    bool Class::isMarkedProtected (const std::string & name) const {
	for (auto & it : this-> _protected) {
	    if (it == name) return true;
	}
	return false;
    }

    void Class::isAbs (bool is) {
	this-> _isAbstract = is;
    }

    bool Class::isAbs () const {
	return this-> _isAbstract;
    }

    void Class::isFinal (bool is) {
	this-> _isFinal = is;
    }

    bool Class::isFinal () const {
	return this-> _isFinal;
    }
    
    void Class::setAddMethods (const std::vector <Symbol> & methods) {
	this-> _addMethods = methods;
    }

    const std::vector <Symbol> & Class::getAddMethods () const {
	return this-> _addMethods;
    }
    
}
