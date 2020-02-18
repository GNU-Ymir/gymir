#include <ymir/semantic/symbol/VarDecl.hh>

namespace semantic {

    VarDecl::VarDecl () :
	ISymbol (lexing::Word::eof (), false),
	_decos (),
	_type (syntax::Expression::empty ()),
	_value (syntax::Expression::empty ()),
	_gen (generator::Generator::empty ())
    {}

    VarDecl::VarDecl (const lexing::Word & name, const std::vector <syntax::DecoratorWord> & decos, const syntax::Expression & type, const syntax::Expression & value, bool isWeak) :
	ISymbol (name, isWeak),
	_decos (decos),
	_type (type),
	_value (value),
	_gen (generator::Generator::empty ())
    {}
    
    Symbol VarDecl::init (const lexing::Word & name, const std::vector <syntax::DecoratorWord> & decos, const syntax::Expression & type, const syntax::Expression & value, bool isWeak) {
	return Symbol {new (Z0) VarDecl (name, decos, type, value, isWeak)};
    }
    
    bool VarDecl::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	VarDecl thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
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
    
    std::string VarDecl::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }
}
