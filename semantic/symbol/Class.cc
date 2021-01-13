#include <ymir/semantic/symbol/Class.hh>
#include <ymir/semantic/symbol/Constructor.hh>
#include <ymir/syntax/expression/Var.hh>

namespace semantic {

    Class::Class () :
	ISymbol (lexing::Word::eof (), "", false), 
	_ancestor (syntax::Expression::empty ()),
	_fields ({}),
	_gen (generator::Generator::empty ()),
	_typeInfo (generator::Generator::empty ())
    {}

    Class::Class (const lexing::Word & name, const std::string & comments, const syntax::Expression & ancestor, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_ancestor (ancestor),
	_fields ({}),
	_gen (generator::Generator::empty ()),
	_typeInfo (generator::Generator::empty ())
    {}

    
    Symbol Class::init (const lexing::Word & name, const std::string & comments, const syntax::Expression & ancestor, bool isWeak) {
	auto ret = Symbol {new (NO_GC) Class (name, comments, ancestor, isWeak)};
	ret.to <Class> ()._table = Table::init (ret.getPtr ());
	return ret;
    }
        
    void Class::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }
    
    void Class::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }

    void Class::getTemplates (std::vector <Symbol> & ret) const {
	auto & tmpls = this-> _table-> getTemplates ();
	ret.insert (ret.begin (), tmpls.begin (), tmpls.end ());
    }    
    
    void Class::replace (const Symbol & sym) {
	this-> _table-> replace (sym);
    }

    void Class::get (const std::string & name, std::vector <Symbol> & ret) const {
	getReferent ().get (name, ret);
    }

    void Class::getPublic (const std::string & name, std::vector <Symbol> & ret) const {
	getReferent ().getPublic (name, ret);
    }
    
    void Class::getLocal (const std::string &, std::vector <Symbol>&) const {
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

    void Class::setFieldComment (const std::string & name, const std::string & comment) {
	this-> _field_comments [name] = comment;
    }
    
    const std::vector<syntax::Expression> & Class::getFields () const {
	return this-> _fields;
    }

    std::string Class::getFieldComments (const std::string & name) const {
	auto it = this-> _field_comments.find (name);
	if (it != this-> _field_comments.end ())
	    return it-> second;
	return "";	
    }

    void Class::addAssertion (const syntax::Expression & assert) {
	this-> _assertions.push_back (assert);
    }

    void Class::addAssertionComments (const std::string & comments) {
	this-> _assertion_comments.push_back (comments);
    }

    const std::vector <syntax::Expression> & Class::getAssertions () const {
	return this-> _assertions;
    }

    const std::vector <std::string> & Class::getAssertionComments () const {
	return this-> _assertion_comments;
    }
    
    const syntax::Expression & Class::getAncestor () const {
	return this-> _ancestor;
    }

    void Class::setPrivate (const std::string & name) {
	this-> _privates.emplace (name);
    }

    void Class::setProtected (const std::string & name) {
	this-> _protected.emplace (name);
    }

    bool Class::isMarkedPrivate (const std::string & name) const {
	if (this-> _privates.find (name) != this-> _privates.end ())
	    return true;	
	return false;
    }
    
    bool Class::isMarkedProtected (const std::string & name) const {
	if (this-> _protected.find (name) != this-> _protected.end ()) 
	    return true;	
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
