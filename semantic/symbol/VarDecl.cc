#include <ymir/semantic/symbol/VarDecl.hh>

namespace semantic {

    VarDecl::VarDecl () :
	ISymbol (lexing::Word::eof (), "", false),
	_decos (),
	_type (syntax::Expression::empty ()),
	_value (syntax::Expression::empty ()),
	_gen (generator::Generator::empty ())
    {}

    VarDecl::VarDecl (const lexing::Word & name, const std::string & comments, const std::vector <syntax::DecoratorWord> & decos, const syntax::Expression & type, const syntax::Expression & value, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_decos (decos),
	_type (type),
	_value (value),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol VarDecl::init (const lexing::Word & name, const std::string & comments, const std::vector <syntax::DecoratorWord> & decos, const syntax::Expression & type, const syntax::Expression & value, bool isWeak) {
	return Symbol {new (NO_GC) VarDecl (name, comments, decos, type, value, isWeak)};
    }
    
    bool VarDecl::equals (const Symbol & other, bool parent) const {
	if (!other.is <VarDecl> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const syntax::Expression & VarDecl::getValue () const {
	return this-> _value;
    }   

    const syntax::Expression & VarDecl::getType () const {
	return this-> _type;
    }
    
    const std::vector <syntax::DecoratorWord> & VarDecl::getDecorators () const {
	return this-> _decos;
    }

    void VarDecl::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }

    const generator::Generator & VarDecl::getGenerator () const {
	return this-> _gen;
    }
    
    void VarDecl::setExternalLanguage (const std::string & name) {
	this-> _externLanguage = name;
    }
	
    const std::string & VarDecl::getExternalLanguage () const {
	return this-> _externLanguage;
    }

    bool VarDecl::isExtern () const {
	return this-> _externLanguage != "";
    }

    std::string VarDecl::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }
}
