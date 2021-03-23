#include <ymir/semantic/symbol/TemplatePreSolution.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/utils/Match.hh>

namespace semantic {


    TemplatePreSolution::TemplatePreSolution () :
	ISymbol (lexing::Word::eof (), "", false),
	_decl (syntax::Declaration::empty ())
    {}

    TemplatePreSolution::TemplatePreSolution (const lexing::Word & loc, const std::string & comments, const std::vector <syntax::Expression> & params, const syntax::Declaration & decls, const std::map <std::string, syntax::Expression> & mapping, const std::vector <std::string> & nameOrder, const Symbol & ref) :
	ISymbol (loc, comments, true),
	_decl (decls),
	_params (params),
	_mapping (mapping),
	_nameOrder (nameOrder),
	_templateRef (ref.getPtr ())
    {}
    
    Symbol TemplatePreSolution::init (const lexing::Word & loc, const std::string & comments, const std::vector <syntax::Expression> & params, const syntax::Declaration & decls, const std::map <std::string, syntax::Expression> & mapping, const std::vector <std::string> & nameOrder, const Symbol & ref) {
	return Symbol {new (NO_GC) TemplatePreSolution (loc, comments, params, decls, mapping, nameOrder, ref)};
    }

    bool TemplatePreSolution::equals (const Symbol & other, bool parent) const {
	if (!other.is <TemplatePreSolution> ()) return false;
	if (other.getName () == this-> getName ()) {
	    if (parent)
		return this-> getReferent ().equals (other.getReferent ());
	    else return false;
	} else return false;
    }

    const syntax::Declaration & TemplatePreSolution::getDeclaration () const {
	return this-> _decl;
    }
        
    const std::vector <syntax::Expression> & TemplatePreSolution::getTemplateParams () const {
	return this-> _params;
    }
    
    const std::vector <std::string> & TemplatePreSolution::getNameOrder () const {
	return this-> _nameOrder;
    }
    
    const std::map <std::string, syntax::Expression> & TemplatePreSolution::getMapping () const {
	return this-> _mapping;
    }

    Symbol TemplatePreSolution::getTemplateReferent () const {
	return Symbol {this-> _templateRef};
    }

    
    std::string TemplatePreSolution::formatTree (int i) const {
	Ymir::OutBuffer buf;
	buf.writefln ("%*- <T> %", i, "|\t", this-> getName ());
	return buf.str ();
    }

}
