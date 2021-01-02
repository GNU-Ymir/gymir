#include <ymir/semantic/symbol/Alias.hh>

namespace semantic {

    Alias::Alias () :
	ISymbol (lexing::Word::eof (), "", false),
	_value (syntax::Expression::empty ()),
	_gen (generator::Generator::empty ())
    {}

    Alias::Alias (const lexing::Word & name, const std::string & comments, const syntax::Expression & value, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_value (value),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol Alias::init (const lexing::Word & name, const std::string & comments, const syntax::Expression & value, bool isWeak) {
	return Symbol {new (NO_GC) Alias (name, comments, value, isWeak)};
    }
    

    bool Alias::equals (const Symbol & other, bool parent) const {
	if (!other.is <Alias> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const syntax::Expression & Alias::getValue () const {
	return this-> _value;
    }   

    void Alias::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }

    const generator::Generator & Alias::getGenerator () const {
	return this-> _gen;
    }
    
    std::string Alias::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }
}
