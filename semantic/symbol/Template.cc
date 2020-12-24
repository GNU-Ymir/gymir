#include <ymir/semantic/symbol/Template.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/utils/Match.hh>

namespace semantic {


    Template::Template () :
	ISymbol (lexing::Word::eof (), false),
	_params ({}),
	_test (syntax::Expression::empty ()),
	_decl (syntax::Declaration::empty ())
    {}

    Template::Template (const lexing::Word & loc, const std::vector<syntax::Expression> & params, const syntax::Declaration & decl, const syntax::Expression & test, const std::vector<syntax::Expression> & previousParams, bool isWeak) :
	ISymbol (loc, isWeak),
	_params (params),
	_test (test),
	_decl (decl),
	_previousParams (previousParams)
    {}
    
    Symbol Template::init (const lexing::Word & loc, const std::vector<syntax::Expression> & params, const syntax::Declaration & decl, const syntax::Expression & test, const std::vector<syntax::Expression> & previousParams, bool isWeak) {
	return Symbol {new (NO_GC) Template (loc, params, decl, test, previousParams, isWeak)};
    }

    bool Template::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Template thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    bool Template::equals (const Symbol & other, bool parent) const {
	if (!other.is <Template> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return false;
	} else return false;
    }

    const syntax::Declaration & Template::getDeclaration () const {
	return this-> _decl;
    }

    const std::vector <syntax::Expression> & Template::getParams () const {
	return this-> _params;
    }

    const std::vector <syntax::Expression> & Template::getPreviousParams () const {
	return this-> _previousParams;
    }

    
    const syntax::Expression & Template::getTest () const {
	return this-> _test;
    }
    
    std::string Template::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- <T> %", i, "|\t", this-> getName ());
	return buf.str ();
    }
    
    void Template::setPreviousSpecialization (const std::map <std::string, syntax::Expression> & previous) {
	this-> _alreadyDone = previous;
    }

    const std::map <std::string, syntax::Expression> & Template::getPreviousSpecialization () const {
	return this-> _alreadyDone;
    }

    const std::vector <std::string> & Template::getSpecNameOrder () const {
	return this-> _nameOrder;
    }

    void Template::setSpecNameOrder (const std::vector <std::string> & nameOrder) {
	this-> _nameOrder = nameOrder;
    }
    
    std::string Template::prettyString () const {
	Ymir::OutBuffer buf;
	buf.writef ("% (", this-> getName ().str);
	for (auto it : Ymir::r (0, this-> _params.size ())) {
	    if (it != 0)
		buf.write (", ");
	    buf.write (this-> _params [it].prettyString ());
	}
	buf.write (")");

	match (this-> _decl) {
	    of (syntax::Function, func, {
		    buf.write ("(");
		    int i = 0;
		    for (auto & it : func.getPrototype ().getParameters ()) {
			if (i != 0) buf.write (", ");
			buf.write (it.prettyString ());
			i += 1;
		    }
		    buf.write (")-> ");		    
		    if (func.getPrototype ().getType ().isEmpty ())
			buf.write ("void");
		    else buf.write (func.getPrototype ().getType ().prettyString ());
		}
	    );
	}
	
	return buf.str ();
    }
    
}
