#include <ymir/semantic/symbol/MacroRule.hh>

namespace semantic {

    MacroRule::MacroRule () :
	ISymbol (lexing::Word::eof (), false),
	_constr (syntax::Declaration::empty ())
    {}
    
    MacroRule::MacroRule (const lexing::Word & name, const syntax::Declaration & constr) :
	ISymbol (name, false),
	_constr (constr)
    {}

    Symbol MacroRule::init (const lexing::Word & name, const syntax::Declaration & constr) {
	return Symbol {new (Z0) MacroRule (name, constr)};
    }

    bool MacroRule::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	MacroRule thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    bool MacroRule::equals (const Symbol & other, bool parent) const {
	if (!other.is <MacroRule> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return true;
	} else return false;
    }

    const syntax::Declaration & MacroRule::getContent () const {
	return this-> _constr;
    }
    
    std::string MacroRule::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- %", i, "|\t", this-> getName ());
	return buf.str ();
    }    

}
