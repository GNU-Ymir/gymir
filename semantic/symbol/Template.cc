#include <ymir/semantic/symbol/Template.hh>

namespace semantic {


    Template::Template () :
	ISymbol (lexing::Word::eof ()),
	_params ({}),
	_test (syntax::Expression::empty ()),
	_decl (syntax::Declaration::empty ())
    {}

    Template::Template (const lexing::Word & loc, const std::vector<syntax::Expression> & params, const syntax::Declaration & decl, const syntax::Expression & test) :
	ISymbol (loc),
	_params (params),
	_test (test),
	_decl (decl)
    {}
    
    Symbol Template::init (const lexing::Word & loc, const std::vector<syntax::Expression> & params, const syntax::Declaration & decl, const syntax::Expression & test) {
	return Symbol {new (Z0) Template (loc, params, decl, test)};
    }

    Symbol Template::clone () const {
	return Symbol {new (Z0) Template (*this)};
    }

    bool Template::isOf (const ISymbol * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Template thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return ISymbol::isOf (type);	
    }

    bool Template::equals (const Symbol & other) const {
	if (!other.is <Template> ()) return false;
	if (other.getName () == this-> getName ()) {
	    return this-> getReferent ().equals (other.getReferent ());
	} else return false;
    }

    const syntax::Declaration & Template::getDeclaration () const {
	return this-> _decl;
    }

    const std::vector <syntax::Expression> & Template::getParams () const {
	return this-> _params;
    }

    const syntax::Expression & Template::getTest () const {
	return this-> _test;
    }
    
    std::string Template::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- <T> %", i, "|\t", this-> getName ());
	return buf.str ();
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
	return buf.str ();
    }
    
}
