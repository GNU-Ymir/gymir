#include <ymir/semantic/symbol/Function.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/syntax/expression/VarDecl.hh>

namespace semantic {

    Function::Function () :
	ISymbol (lexing::Word::eof ()),
	_table (this),
	_content (syntax::Declaration::empty ())
    {}

    Function::Function (const lexing::Word & name, const syntax::Function & func) :
	ISymbol (name),
	_table (this),
	_content (syntax::Declaration {func.clone ()}),
	_isVariadic (func.getPrototype ().isVariadic ()),
	_isOver (func.isOver ())
    {}

    Function::Function (const Function & other) :
	ISymbol (other),
	_table (other._table.clone (this)), 
	_content (other._content),
	_isVariadic (other._isVariadic),
	_isOver (other._isOver)
    {}
    
    Symbol Function::init (const lexing::Word & name, const syntax::Function & func) {
	return Symbol {new Function (name, func)};
    }

    bool Function::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Function thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);
    }

    void Function::insert (const Symbol & sym) {
	this-> _table.insert (sym);
    }

    void Function::insertTemplate (const Symbol & sym) {
	this-> _table.insertTemplate (sym);
    }
    
    std::vector<Symbol> Function::getTemplates () const {
	return this-> _table.getTemplates ();
    }    
    
    void Function::replace (const Symbol & sym) {
	this-> _table.replace (sym);
    }

    std::vector<Symbol> Function::get (const std::string & name) const {
	auto vec = getReferent ().get (name);
	auto local = this-> _table.get (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }

    std::vector<Symbol> Function::getPublic (const std::string & name) const {
	auto vec = getReferent ().getPublic (name);
	auto local = this-> _table.getPublic (name);
	vec.insert (vec.begin (), local.begin (), local.end ());
	return vec;
    }
    
    std::vector <Symbol> Function::getLocal (const std::string & name) const {
	return this-> _table.get (name);
    }

    std::vector <Symbol> Function::getLocalPublic (const std::string & name) const {
	return this-> _table.getPublic (name);
    }
    
    
    bool Function::equals (const Symbol & other, bool parent) const {
	if (!other.is <Function> ()) return false;
	if (other.getName ().isSame (this-> getName ())) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else
	    return false;
    }

    void Function::isPure (bool is) {
	this-> _isPure = is;
    }

    void Function::isInline (bool is) {
	this-> _isInline = is;
    }

    void Function::isSafe (bool is) {
	this-> _isSafe = is;
    }

    void Function::isOver (bool is) {
	this-> _isOver = is;
    }

    bool Function::isOver () const {
	return this-> _content.to <syntax::Function> ().isOver ();
    }

    bool Function::isMethod () const {
	auto & proto = this-> _content.to <syntax::Function> ().getPrototype ();
	if (proto.getParameters ().size () >= 1) {
	    if (proto.getParameters ()[0].is <syntax::VarDecl> () &&
		proto.getParameters ()[0].to <syntax::VarDecl> ().getType ().isEmpty () &&
		proto.getParameters ()[0].to <syntax::VarDecl> ().getName () == Keys::SELF)
		return true;
	}
	return false;
    }
    
    const syntax::Function & Function::getContent () const {
	return this-> _content.to <syntax::Function> ();
    }

    std::string Function::getRealName () const {
	if (this-> getName ().str == Keys::MAIN) return this-> getName ().str;
	else return ISymbol::getRealName ();
    }

    std::string Function::getMangledName () const {
	if (this-> getName ().str == Keys::MAIN) return this-> getName ().str;
	else return ISymbol::getMangledName ();
    }
    
    void Function::setExternalLanguage (const std::string & name) {
	this-> _externLanguage = name;
    }

    const std::string & Function::getExternalLanguage () const {
	return this-> _externLanguage;
    }

    std::string Function::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	for (auto & it : this-> _table.getAll ()) {
	    buf.write (it.formatTree (i + 1));
	}
	return buf.str ();
    }

    bool Function::isVariadic () const {
	return this-> _isVariadic;
    }

    void Function::isFinal (bool is) {
	this-> _isFinal = is;
    }

    bool Function::isFinal () const {
	return this-> _isFinal;
    }
    
}
