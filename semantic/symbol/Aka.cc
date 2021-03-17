#include <ymir/semantic/symbol/Aka.hh>

namespace semantic {

    Aka::Aka () :
	ISymbol (lexing::Word::eof (), "", false),
	_value (syntax::Expression::empty ()),
	_gen (generator::Generator::empty ())
    {}    

    Aka::Aka (const lexing::Word & name, const std::string & comments, const syntax::Expression & value, bool isWeak) :
	ISymbol (name, comments, isWeak),
	_value (value),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol Aka::init (const lexing::Word & name, const std::string & comments, const syntax::Expression & value, bool isWeak) {
	return Symbol {new (NO_GC) Aka (name, comments, value, isWeak)};
    }
    

    bool Aka::equals (const Symbol & other, bool parent) const {
	if (!other.is <Aka> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const syntax::Expression & Aka::getValue () const {
	return this-> _value;
    }   

    void Aka::setGenerator (const generator::Generator & gen) {
	this-> _gen = gen;
    }

    const generator::Generator & Aka::getGenerator () const {
	return this-> _gen;
    }
    
    std::string Aka::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }
}
