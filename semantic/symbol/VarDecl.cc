#include <ymir/semantic/symbol/VarDecl.hh>

namespace semantic {

    VarDecl::VarDecl () :
	ISymbol (lexing::Word::eof ()),
	_decos (),
	_type (syntax::Expression::empty ()),
	_value (syntax::Expression::empty ())
    {}

    VarDecl::VarDecl (const lexing::Word & name, const std::vector <syntax::Decorator> & decos, const syntax::Expression & type, const syntax::Expression & value) :
	ISymbol (name),
	_decos (decos),
	_type (type),
	_value (value)
    {}

    Symbol VarDecl::clone () const {
	return VarDecl::init (this-> getName (), this-> _decos, this-> _type, this-> _value);
    }
    
    Symbol VarDecl::init (const lexing::Word & name, const std::vector <syntax::Decorator> & decos, const syntax::Expression & type, const syntax::Expression & value) {
	return Symbol {new (Z0) VarDecl (name, decos, type, value)};
    }
    
    bool VarDecl::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	VarDecl thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    bool VarDecl::equals (const Symbol & other) const {
	if (!other.is <VarDecl> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

    const syntax::Expression & VarDecl::getValue () const {
	return this-> _value;
    }   

    const syntax::Expression & VarDecl::getType () const {
	return this-> _type;
    }
    
}
