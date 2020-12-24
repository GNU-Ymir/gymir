#pragma once

#include <ymir/syntax/expression/Match.hh>
#include <ymir/syntax/visitor/Visitor.hh>
#include <ymir/syntax/declaration/MacroConstructor.hh>
#include <ymir/semantic/validator/Visitor.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct MatchVisitor
	 * This visitor is responsible of the validation of the match operator
	 */
	class MacroVisitor {

	    static const std::string __EXPR__;

	    static const std::string __IDENT__;

	    static const std::string __TOKEN__;
	    
	private : 

	    Visitor & _context;

	    std::string _content;

	    lexing::Word _call;
	    
	public :
	    
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

		std::string toString (int i = 0) const;		    		
		
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

	    /**
	     * \brief Validate a Macro call when there are multiple symbols with the same name
	     * \brief Browse the symbol and call validateMacroRef when the symbol is a MacroRef
	     */
	    generator::Generator validateMultSym (const generator::MultSym & sym, const syntax::MacroCall & expression, std::vector <std::string> & errors);

	    /**
	     * Validate a macro call on a macro ref symbol
	     * Actually expand the macro call on the ref and produce the results
	     */
	    generator::Generator validateMacroRef (const generator::MacroRef & mref, const syntax::MacroCall & expression, std::vector <std::string> & errors);

	    /**
	     * Validate the constructor of a macro ref, there is only one construtor by macro
	     */
	    generator::Generator validateConstructor (const semantic::Symbol & constr, const syntax::MacroCall & expression);

	    /**
	     * Validate a macro mutliplier expression (*|+|?)
	     * \param content, the remaining of the macro call 
	     * \param current, the rest of the content of the macro when the multiplier has finished
	     * \param skips, the skipable token (when no other possibily is available)
	     */
	    Mapper validateMacroMult (const syntax::MacroMult & mult, const std::string & content, ulong & current, const syntax::Expression &  skips);

	    /**
	     * Validate a list of macro expression
	     * This is called by macro mult when the mutliplier is just once
	     * \param content, the remaining of the macro call 
	     * \param current, the rest of the content of the macro when the multiplier has finished
	     * \param skips, the skipable token (when no other possibily is available)
	     */
	    Mapper validateMacroList (const syntax::MacroMult & mult, const std::string & content, ulong & current, const syntax::Expression &  skips);

	    /**
	     * Validate a list of macro repeat expression
	     * This is called by macro mult when the multiplier is as many as there are
	     * \param content, the remaining of the macro call 
	     * \param current, the rest of the content of the macro when the multiplier has finished
	     * \param skips, the skipable token (when no other possibily is available)
	     */
	    std::vector <Mapper> validateMacroRepeat (const syntax::MacroMult & mult, const std::string & content, ulong & current, const syntax::Expression &  skips);

	    /**
	     * Validate a list of macro repeat expression
	     * This is called by macro mult when the multiplier is as many as there are but at least one
	     * \param content, the remaining of the macro call 
	     * \param current, the rest of the content of the macro when the multiplier has finished
	     * \param skips, the skipable token (when no other possibily is available)
	     */
	    std::vector<Mapper> validateMacroOneOrMore (const syntax::MacroMult & mult, const std::string & content, ulong & current, const syntax::Expression &  skips);

	    /**
	     * Validate a list of macro repeat expression
	     * This is called by macro mult when the multiplier is one or zero 
	     * \param content, the remaining of the macro call 
	     * \param current, the rest of the content of the macro when the multiplier has finished
	     * \param skips, the skipable token (when no other possibily is available)
	     */	     
	    Mapper validateMacroOneOrNone (const syntax::MacroMult & mult, const std::string & content, ulong & current, const syntax::Expression &  skips);

	    /**
	     * Validate a or expression
	     * A or expression is a list of token, and only one is taken
	     * \param content, the remaining of the macro call 
	     * \param current, the rest of the content of the macro when the multiplier has finished 
	     * \param skips, the skipable token (when no other possibily is available)
	     */
	    Mapper validateMacroOr (const syntax::MacroOr & mult, const std::string & content, ulong & current, const syntax::Expression &  skips);

	    /**
	     * Validate a macro var
	     * A macro will affect the result of a macro rule (or a macro expression) inside a variable
	     * \param content, the remaining of the macro call 
	     * \param current, the rest of the content of the macro when the multiplier has finished (cursor)
	     * \param skips, the skipable token (when no other possibily is available)	     
	     */
	    Mapper validateMacroVar (const syntax::MacroVar & mult, const std::string & content, ulong & current, const syntax::Expression &  skips);

	    /**
	     * Validate a simple macro token
	     * \param content, the remaining of the macro call 
	     * \param current, the rest of the content of the macro when the multiplier has finished 	     
	     * \param skips, the skipable token (when no other possibily is available)
	     */
	    Mapper validateMacroToken (const syntax::MacroToken & mult, const std::string & content, ulong & current, const syntax::Expression &  skips);

	    /**
	     * Validate a macro rule 
	     * A rule is a part of a MacroRef, it contains macros expression and a content
	     */
	    Mapper validateRule (const syntax::Expression & expr, const std::string & content, ulong & current, const syntax::Expression &  skips);


	    /**
	     * Validate the content of the skips
	     * It will return a macro mult containing macro ors
	     */
	    syntax::Expression validateSkips (const std::vector <syntax::Expression> & skips) const;
	    
	    /**
	     * Merge two mappers 
	     * \return a mapper, if the merging failed, the mapper is marked as a failure
	     */
	    Mapper mergeMapper (const Mapper & left, const Mapper & right) const ;


	    /**
	     * Validate a string, with macro inside it from a valid mapper
	     * \return the string that has been produced
	     */
	    std::string validateMapperString (const lexing::Word& loc, const std::string & expr, const Mapper & mapping);
	    
	    /**
	     * Validate a string content with a mapper
	     * This will traverse the whole string and change for each mapping element the associated result
	     * This is done by calling the function validateMacroEval
	     */
	    syntax::Expression validateMapper (const lexing::Word & loc, const std::string & expr, const Mapper & mapping);

	    /**
	     * This function transform a string into a MacroVal expression
	     */
	    syntax::Expression toMacroEval (const std::string & content, const lexing::Word & loc, lexing::Lexer & lex, const lexing::Word & begin);

	    /**
	     * Validate a macro eval expression
	     * \return the result mappers found from the execution of the expression
	     */
	    std::vector<Mapper> validateMacroEval (const std::string & content, const lexing::Word & loc, const syntax::Expression & eval, const Mapper & mapper);

	    /**
	     * Validate a macro for expression that loop over a Mapper
	     */
	    std::string validateMacroFor (const std::string & content, const lexing::Word & loc, lexing::Lexer & lex, const Mapper & mapping);


	    
	    
	    void computeLine (ulong & line, ulong & col, ulong & seek, ulong current);

	    void error (const lexing::Word & location, const lexing::Word & end, const generator::Generator & left, const std::vector <std::string> & errors);
	    
	};
	
    }
    
}
