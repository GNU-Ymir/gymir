#include <ymir/semantic/symbol/MacroConstructor.hh>

namespace semantic {

    MacroConstructor::MacroConstructor () :
	ISymbol (lexing::Word::eof (), false),
	_constr (syntax::Declaration::empty ())
    {}
    
    MacroConstructor::MacroConstructor (const lexing::Word & name, const syntax::Declaration & constr) :
	ISymbol (name, false),
	_constr (constr)
    {}

    Symbol MacroConstructor::init (const lexing::Word & name, const syntax::Declaration & constr) {
	return Symbol {new (Z0) MacroConstructor (name, constr)};
    }
    
    bool MacroConstructor::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroConstructor thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    bool MacroConstructor::equals (const Symbol & other, bool parent) const {
	if (!other.is <MacroConstructor> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const syntax::Declaration & MacroConstructor::getContent () const {
	return this-> _constr;
    }
    
    std::string MacroConstructor::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }    

}