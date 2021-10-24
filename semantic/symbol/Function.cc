#include <ymir/semantic/symbol/Function.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/syntax/expression/VarDecl.hh>

namespace semantic {

    Function::Function () :
	ISymbol (lexing::Word::eof (), "", false),
	_content (syntax::Declaration::empty ()),
	_proto (generator::Generator::empty ())
    {}

    Function::Function (const lexing::Word & name, const std::string & comments, const syntax::Function & func, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_content (syntax::Function::init (func)),
	_proto (generator::Generator::empty ()),
	_isVariadic (func.getPrototype ().isVariadic ()),
	_isOver (func.isOver ())
    {}

    Symbol Function::init (const lexing::Word & name, const std::string & comments, const syntax::Function & func, bool isWeak) {
	auto ret = Symbol {new (NO_GC) Function (name, comments, func, isWeak)};
	ret.to <Function> ()._table = Table::init (ret.getPtr ());
	return ret;
    }

    void Function::insert (const Symbol & sym) {
	this-> _table-> insert (sym);
    }

    void Function::insertTemplate (const Symbol & sym) {
	this-> _table-> insertTemplate (sym);
    }
    
    void Function::getTemplates (std::vector <Symbol> & rets) const {
	auto & tmpls = this-> _table-> getTemplates ();
	rets.insert (rets.end (), tmpls.begin (), tmpls.end ());
    }    
    
    // void Function::replace (const Symbol & sym) {
    // 	this-> _table-> replace (sym);
    // }

    void Function::get (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().get (name, rets);
	this-> _table-> get (name, rets);
    }

    void Function::getPublic (const std::string & name, std::vector <Symbol> & rets) const {
	getReferent ().getPublic (name, rets);
	this-> _table-> getPublic (name, rets);
    }
    
    void Function::getLocal (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> get (name, rets);
    }

    void Function::getLocalPublic (const std::string & name, std::vector <Symbol> & rets) const {
	this-> _table-> getPublic (name, rets);
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

    bool Function::isSafe () const {
	return this-> _isSafe;
    }

    void Function::isOver (bool is) {
	this-> _isOver = is;
    }

    void Function::isTest (bool is) {
	this-> _isTest = is;
    }

    bool Function::isTest () const {
	return this-> _isTest;
    }

    bool Function::isOver () const {
	return this-> _content.to <syntax::Function> ().isOver ();
    }

    void Function::setThrowers (const std::vector <syntax::Expression> & throwers) {
	this-> _throwers = throwers;
    }

    const std::vector <syntax::Expression> & Function::getThrowers () const {
	return this-> _throwers;
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

    std::string Function::computeRealName () const {
	if (this-> getName () == Keys::MAIN) return this-> getName ().getStr ();
	else return ISymbol::computeRealName ();
    }

    std::string Function::computeMangledName () const {
	if (this-> getName () == Keys::MAIN) return this-> getName ().getStr ();
	else return ISymbol::computeMangledName ();
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
	for (auto & it : this-> _table-> getAll ()) {
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


    const generator::Generator & Function::getGenerator () const {
	return this-> _proto;
    }

    void Function::setGenerator (const generator::Generator & gen) {
	this-> _proto = gen;
    }
    
}
