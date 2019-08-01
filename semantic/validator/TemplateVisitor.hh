#pragma once

#include <ymir/syntax/declaration/Template.hh>
#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/generator/value/Binary.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>

namespace semantic {

    namespace validator {

	/**
	 * \struct TemplateVisitor
	 * This visitor is used to validate template specialisation 
	 * A template can be specialized implicitly by call operator (x,...) 
	 * Or explicitly by template operator !(x,...)
	 */
	class TemplateVisitor {

	    enum Scores {
		SCORE_VAR = 1,
		SCORE_TYPE = 2
	    };
	    
	    struct Mapper {
		bool succeed;
		uint score;
		std::map <std::string, syntax::Expression> mapping;
	    };		
	    
	    Visitor & _context;


	private :
	    
	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    TemplateVisitor (Visitor & context);


	public :

	    /**
	     * \brief Create a template visitor 
	     * \param context the context of the visitor (used to validate some type and expressions)
	     */
	    static TemplateVisitor init (Visitor & context);

	    /**
	     * \brief Validate Template expression from implicit specialistion (call operator) 
	     * \param ref the reference of the template declaration
	     * \param valueParams the parameters passed at the template 
	     * \param types the types of the parameters of the implicit call
	     * \param finalParams by return reference, the parameters to pass at the template symbol for final validation after template specialization
	     */
	    generator::Generator validateFromImplicit (const generator::TemplateRef & ref, const std::vector <generator::Generator> & valueParams, const std::vector <generator::Generator> & types, int & score, Symbol & sym, std::vector <generator::Generator> & finalParams) const;


	    /**
	     * \brief Validate a type template specialisation from implicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param left the argument basically a var decl -> x : R
	     * \param type the type used for implicit specialization
	     * \return a Mapper, which stores the modifications to apply
	     */
	    Mapper validateVarDeclFromImplicit (const std::vector <syntax::Expression> & params, const syntax::Expression & left, const std::vector <generator::Generator> & types, int & consumed) const;

	    /**
	     * \brief Validate a type template specialization from implicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param params the type of the argument can be many things (var, list, ...)
	     * \param params type the type used for implicit specialization
	     * \return a Mapper, which stores the modifications to apply
	     */
	    Mapper validateTypeFromImplicit (const std::vector <syntax::Expression> & params, const syntax::Expression & left, const std::vector <generator::Generator> & types, int & consumed) const;


	    /**
	     * \brief Validate a type template specialization from explicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param left one of the template parameters \in params
	     * \param type the type that will make the specialization
	     */
	    Mapper applyTypeFromExplicit (const std::vector <syntax::Expression> & params, const syntax::Expression & left, const std::vector <generator::Generator> & types, int & consumed) const;

	    /**
	     * \brief Validate a type template specialization from explicit context on an OfVar
	     * \param params the template parameters (T : [R], R, ...)
	     * \param left one of the template parameters \in params, which is a OfVar
	     * \param type the type that will make the specialization
	     */
	    Mapper applyTypeFromExplicitOfVar (const std::vector <syntax::Expression> & params, const syntax::OfVar & var, const generator::Generator & types) const;

	    /**
	     * \brief Find the expression named name in the list of params (direct access)
	     * \param name the name of the expression to find
	     * \param params the template parameters (T : [R], R, ...)
	     */
	    syntax::Expression findExpression (const std::string & name, const std::vector <syntax::Expression> & params) const;
	    
	    /**
	     * \brief Create syntax tree from generator type
	     * \brief Reverse the compilation
	     */
	    syntax::Expression createSyntaxType (const lexing::Word & loc, const generator::Generator & type) const;

	private :

	    /**
	     * \brief Merge two mappers 
	     * \warning assume that the merge will succed (meaning there is no conflict between left and right)
	     * \warning if there are conflicts, throw an internal error
	     * \param left a mapper
	     * \param right a mapper
	     */
	    Mapper mergeMappers (const Mapper & left, const Mapper & right) const;
	    
	    /**
	     * \brief Transform an expression using a template mapper
	     * \param element the syntax element to transform
	     * \param mapping the mapper element of the template specialization
	     */
	    syntax::Expression replaceAll (const syntax::Expression & element, const std::map <std::string, syntax::Expression> & mapping) const;

	    /**
	     * \brief Transform a declaration using a template mapper
	     * \param element the declaration to transform
	     * \param mapping the mapper element of the template specialization
	     */
	    syntax::Declaration replaceAll (const syntax::Declaration & element, const std::map <std::string, syntax::Expression> & mapping) const;

	    syntax::Function::Prototype replaceAll (const syntax::Function::Prototype & element, const std::map <std::string, syntax::Expression> & mapping) const;

	    syntax::Function::Body replaceAll (const syntax::Function::Body & element, const std::map <std::string, syntax::Expression> & mapping) const;


	    /**
	     * \brief Update the template parameters using a mapper generated
	     * \param elements the template parameters specialization
	     * \param mapping the mapper of the template specialization
	     */
	    std::vector <syntax::Expression> replaceSyntaxTempl (const std::vector <syntax::Expression> & elements, const std::map <std::string, syntax::Expression> & mapping) const;
	    
	};

    }

}
