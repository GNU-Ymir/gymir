#pragma once

#include <ymir/syntax/expression/Match.hh>
#include <ymir/syntax/declaration/MacroConstructor.hh>
#include <ymir/semantic/validator/Visitor.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct MatchVisitor
	 * This visitor is responsible of the validation of the match operator
	 */
	class MacroVisitor {

	    Visitor & _context;

	    struct Mapper {

		bool succeed;
		std::string consumed;
		std::map <std::string, std::vector <Mapper>> mapping;

		Mapper (bool succeed) {
		    this-> succeed = succeed;
		}

		Mapper (bool succeed, std::string consumed) {
		    this-> succeed = succeed;
		    this-> consumed = consumed;
		}
		
		Mapper (bool succeed, std::string consumed, std::map <std::string, std::vector<Mapper> > mapping) {
		    this-> succeed = succeed;
		    this-> consumed = consumed;
		    this-> mapping = mapping;
		}
		
	    };	   
	    
	private :
	    
	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    MacroVisitor (Visitor & context);

	public : 

	    /**
	     * \brief Create a new Visitor
	     * \param context the context of the match visitor (used to validate the operands)
	     */
	    static MacroVisitor init (Visitor & context);
	    
	    /**
	     * \brief Validate a macro expression and return a generator
	     */
	    generator::Generator validate (const syntax::MacroCall & expression);

	    generator::Generator validateMultSym (const generator::MultSym & sym, const syntax::MacroCall & expression, std::vector <std::string> & errors);
	    
	    generator::Generator validateMacroRef (const generator::MacroRef & mref, const syntax::MacroCall & expression, std::vector <std::string> & errors);
	    
	    generator::Generator validateConstructor (const semantic::MacroConstructor & constr, const syntax::MacroCall & expression);

	    Mapper validateMacroMult (const syntax::MacroMult & mult, const std::string & content, std::string & rest);

	    Mapper validateMacroList (const syntax::MacroMult & mult, const std::string & content, std::string & rest);
	    
	    Mapper validateMacroRepeat (const syntax::MacroMult & mult, const std::string & content, std::string & rest);
	    Mapper validateMacroOneOrMore (const syntax::MacroMult & mult, const std::string & content, std::string & rest);
	    Mapper validateMacroOneOrNone (const syntax::MacroMult & mult, const std::string & content, std::string & rest);

	    Mapper validateMacroOr (const syntax::MacroOr & mult, const std::string & content, std::string & rest);

	    Mapper validateMacroVar (const syntax::MacroVar & mult, const std::string & content, std::string & rest);

	    Mapper validateMacroToken (const syntax::MacroToken & mult, const std::string & content, std::string & rest);	    
	    Mapper mergeMapper (const Mapper & left, const Mapper & right) const ;

	    syntax::Expression validateMapper (const syntax::Expression & expr, const Mapper & mapping);
	    
	    generator::Generator validateExpression (const syntax::Expression & expr);	    

	    syntax::Expression validateMacroEval (const syntax::MacroEval & eval, const Mapper & mapper);	    
	    
	    void error (const lexing::Word & location, const lexing::Word & end, const generator::Generator & left, const std::vector <std::string> & errors);
	    
	};
	
    }
    
}
