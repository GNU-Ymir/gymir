#pragma once

#include <ymir/syntax/declaration/Template.hh>
#include <ymir/syntax/declaration/CondBlock.hh>
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

	    // We can't make a slice of a vector in C++, because i don't know
	    // This is an implementation of slice, to remove multiple useless copies
	    template <typename T>
	    class array_view {

		typename std::vector <T>::const_iterator begin_;
		
		typename std::vector <T>::const_iterator end_;
		
	    public:

		array_view (const std::vector <T> & v) :
		    array_view (v.begin (), v.end ()) {}
		
		array_view(const typename std::vector <T>::const_iterator begin, const typename std::vector <T>::const_iterator& end): begin_(begin), end_(end) {}

		array_view (const array_view<T> & other) :
		    array_view (other.begin_, other.end_) {}

		const array_view<T> & operator= (const array_view<T> & other) {
		    this-> begin_ = other.begin_;
		    this-> end_ = other.end_;
		    return other;
		}
		
		typename std::vector <T>::const_iterator begin () const {
		    return begin_;
		}

		typename std::vector <T>::const_iterator end () const {
		    return end_;
		}
		
		uint32_t size () const {
		    return end_ - begin_;
		}
		
		const T & operator[](std::size_t index) const {
		    return *(this->begin_ + index);
		}
		
	    };
	    
	    enum Scores {
		SCORE_VAR = 1,
		SCORE_TYPE = 2
	    };

	    /**
	     * The mapper class associates var identifier to expression
	     * A mapper is constructed by a template resolution, and is then used by template replacement to create a syntax tree with the specialized templates
	     * The mapper has a score (set at template specialization time) to be able to sort template resolutions
	     */
	    struct Mapper {
		bool succeed;
		uint32_t score;
		std::map <std::string, syntax::Expression> mapping;
		std::vector <std::string> nameOrder;

		Mapper (bool succ, uint32_t score) {
		    this-> succeed = succ;
		    this-> score = score;
		}

		Mapper (bool succ, uint32_t score, const std::map <std::string, syntax::Expression> & mapping, const std::vector <std::string> & names) {
		    this-> succeed = succ;
		    this-> score = score;
		    this-> mapping = mapping;
		    this-> nameOrder = names;
		}
		
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
	     * ================================================================================
	     * ================================================================================
	     * =================================     INFERENCE    =============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * Validate the template parameters without argument list
	     * This validation works for templates that have only one valid specialization
	     * If the template has more than one specialization, it returns a empty/false Mapper
	     * @nothrow
	     */
	    Mapper inferArguments (const std::vector <syntax::Expression> & params) const;

	    /**
	     * Infer the value or type of a template parameter
	     * @params:
	     *    - rest: the list of template parameters of the template (minus the validated ones, and param)
	     *    - param: the first template parameter that is not validated yet
	     * @throws: when the inference failed, or some kind of validation failed (it can be anything, we call context.validateValue without catch in here)
	     * */
	    Mapper inferArgumentParamTempl (const array_view <syntax::Expression> & rest, const syntax::Expression & param) const;

	    /**
	     * Infer the type from an of var, and populate the mapper
	     * @throw: we call context.validateType, so everything can be thrown
	     * */
	    Mapper inferTypeFromOfVar (const array_view <syntax::Expression> & rest, const syntax::OfVar & param) const;

	    /*
	     * Infer the value of a template parameter from a var declaration and Populate the mapper
	     * @throw: we call context.validateType, so everything can be thrown
	     * */
	    Mapper inferValueFromVarDecl (const array_view <syntax::Expression> & rest, const syntax::VarDecl & param) const;


	    /**
	     * Infer the value of a template parameter from an inner validation, 
	     * inner means that a sub parameter (such as the type of an ofvar, of a vardecl, etc.), of a parameter decls
	     * @throw: we call context.validateType, so everything can be thrown
	     */
	    Mapper inferArgumentInner (const array_view<syntax::Expression> & rest, const syntax::Expression & elem) const;

	    
	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================     EXPLICIT    ==============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     * \brief Validate a template from template call explicit specialization
	     * \param params the parameter of the template
	     * \param valls the parameter pass to the call
	     */
	    Mapper validateFromExplicit (const std::vector <syntax::Expression> & params, const std::vector <generator::Generator> & calls, int & consumed) const;
	    
	    /**
	     * \brief Validate a template from template call explicit specialization
	     * \param ref the template reference used in the call
	     * \param params the parameters passed at the call
	     */
	    semantic::Symbol validateFromExplicit (const generator::TemplateRef & ref, const std::vector <generator::Generator> & params, int & score) const;

	    /**
	     * Apply a successful mapper on a symbol to generate the TemplatePreSolution (or another template if the validation is not full)
	     */
	    semantic::Symbol applyMapperOnTemplate (const lexing::Word & loc, const semantic::Symbol & sym, const Mapper & mapper, int & score) const;
	    

	    Mapper validateParamTemplFromExplicit (const array_view <syntax::Expression> & paramTempl, const syntax::Expression & param, const array_view <generator::Generator> & values, int & consumed) const;


	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================     IMPLICIT    ==============================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    /**
	     * \brief Validate Template expression from implicit specialistion (call operator)
	     * \param ref the reference of the template declaration
	     * \param valueParams the parameters passed at the template
	     * \param types the types of the parameters of the implicit call
	     * \param finalParams by return reference, the parameters to pass at the template symbol for final validation after template specialization
	     */
	    generator::Generator validateFromImplicit (const generator::Generator & ref, const std::vector <generator::Generator> & valueParams, const std::vector <generator::Generator> & types, int & score, Symbol & sym, std::vector <generator::Generator> & finalParams) const;


	    generator::Generator validateFromImplicit (const generator::Generator & ref, const std::vector <generator::Generator> & valueParams, const std::vector <generator::Generator> & types, const std::vector <syntax::Expression> & syntaxParams, int & score, Symbol & sym, std::vector <generator::Generator> & finalParams) const;


	    /**
	     * \brief Validate a type template specialisation from implicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param left the argument basically a var decl -> x : R
	     * \param type the type used for implicit specialization
	     * \return a Mapper, which stores the modifications to apply
	     */
	    Mapper validateVarDeclFromImplicit (const array_view <syntax::Expression> & params, const syntax::Expression & left, const array_view <generator::Generator> & types, int & consumed) const;

	    /**
	     * \brief Validate a type template specialization from implicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param params the type of the argument can be many things (var, list, ...)
	     * \param params type the type used for implicit specialization
	     * \return a Mapper, which stores the modifications to validate
	     */
	    Mapper validateTypeFromImplicit (const array_view <syntax::Expression> & params, const syntax::Expression & left, const array_view <generator::Generator> & types, int & consumed) const;


	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================       TYPES      =============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * \brief Validate a type template specialization from implicit context
	     * \param params the template parameters
	     * \param cl a template call from a vardecl, (a : X!(T))
	     * \param type the type associated to this vardecl for implicit specialization
	     * \return a Mapper, which store the modification to validate
	     */
	    Mapper validateTypeFromTemplCall (const array_view <syntax::Expression> & params, const syntax::TemplateCall & cl, const generator::Generator & type, int & consumed) const;
	    
	    /**
	     * \brief Validate a type template specialization from implicit context
	     * \param params the template parameters
	     * \param cl a template call from a vardecl, (a : X!(T))
	     * \param the symbol of the template solution
	     * \return a Mapper, which store the modification to validate
	     */
	    Mapper validateTypeFromTemplCall (const array_view <syntax::Expression> & params, const syntax::TemplateCall & cl, const semantic::TemplateSolution & tmplS, int & consumed) const;

	    /**
	     * \brief Validate a type template specialization from explicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param left one of the template parameters \in params
	     * \param type the type that will make the specialization
	     */
	    Mapper validateTypeFromExplicit (const array_view <syntax::Expression> & params, const syntax::Expression & left, const array_view <generator::Generator> & types, int & consumed) const;

	    /**
	     * \brief Validate a type template specialization from explicit context on an OfVar
	     * \param params the template parameters (T of [R], R, ...)
	     * \param left one of the template parameters \in params, which is a OfVar
	     * \param type the type that will make the specialization
	     */
	    Mapper validateTypeFromExplicitOfVar (const array_view <syntax::Expression> & params, const syntax::OfVar & var, const generator::Generator & types) const;

	    /**
	     * \brief Validate a type template specialization from explicit context on an ImplVar
	     * \param params the template parameters (T impl isTruc!R, R, ...)
	     * \param left one of the template parameters \in params, which is a ImplVar
	     * \param type the type that will make the specialization
	     */
	    Mapper validateTypeFromExplicitImplVar (const array_view <syntax::Expression> & params, const syntax::ImplVar & implvar, const generator::Generator & types) const;


	    /**
	     * \brief Validate a decorated expression specialization from implicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param expr the decorated expression
	     */
	    Mapper validateTypeFromDecoratedExpression (const array_view <syntax::Expression> & params, const syntax::DecoratedExpression & expr, const array_view <generator::Generator> & types, int & consumed) const;


	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================       FINDS      =============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * \brief Find the expression named name in the list of params (direct access)
	     * \param name the name of the expression to find
	     * \param params the template parameters (T : [R], R, ...)
	     */
	    syntax::Expression findExpression (const std::string & name, const array_view <syntax::Expression> & params) const;

	    /**
	     * \brief Find the expression named name in the list of params (direct access)
	     * \param name the name of the expression to find
	     * \param params the template parameters (T : [R], R, ...)
	     */
	    syntax::Expression findExpressionValue (const std::string & name, const array_view <syntax::Expression> & params) const;

	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================      CREATES     =============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * \brief Create syntax tree from generator type
	     * \brief Reverse the compilation (kind of, it just return a SyntaxWrapper for a generator)
	     */
	    syntax::Expression createSyntaxType (const lexing::Word & loc, const generator::Generator & type// , bool isMutable = false, bool isRef = false
		) const;
	    
	    /**
	     * \brief Create syntax tree from generator type
	     * \brief Reverse the compilation (kind of, it just return a SyntaxWrapper for a generator)
	     */
	    syntax::Expression createSyntaxType (const lexing::Word & loc, const array_view <generator::Generator> & types) const;


	    /**
	     * \brief Create syntax tree from generator value
	     * \brief Reverse the compilation (kind of, it just return a SyntaxWrapper for a generator)
	     */
	    syntax::Expression createSyntaxValue (const lexing::Word & loc, const generator::Generator & type) const;

	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================      MAPPERS     =============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * \brief Merge two mappers
	     * \warning assume that the merge will succed (meaning there is no conflict between left and right)
	     * \warning if there are conflicts, throw an internal error
	     * \param left a mapper
	     * \param right a mapper
	     */
	    Mapper mergeMappers (const Mapper & left, const Mapper & right) const;

	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================      REPLACE    ==============================
	     * ================================================================================
	     * ================================================================================
	     */

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
	    syntax::Declaration replaceAll (const syntax::Declaration & element, const std::map <std::string, syntax::Expression> & mapping, const semantic::Symbol & ref) const;

	    syntax::Function::Prototype replaceAll (const syntax::Function::Prototype & element, const std::map <std::string, syntax::Expression> & mapping) const;

	    /**
	     * Cond blocks are declared inside a class
	     */
	    syntax::Declaration replaceAll (const Symbol & _ref, const syntax::CondBlock & decl, const std::map <std::string, syntax::Expression> & mapping) const;
	    
	    /**
	     * \brief Update the template parameters using a mapper generated
	     * \param elements the template parameters specialization
	     * \param mapping the mapper of the template specialization
	     */
	    std::vector <syntax::Expression> replaceSyntaxTempl (const std::vector <syntax::Expression> & elements, const std::map <std::string, syntax::Expression> & mapping) const;


	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================      SOLUTIONS    ============================
	     * ================================================================================
	     * ================================================================================
	     */

	    /**
	     * \brief Return the first templatesolution in a tree of symbol (the closest to the root)
	     * \param symbol the symbol to test
	     */
	    semantic::Symbol getFirstTemplateSolution (const semantic::Symbol & symbol) const;


	    Symbol getTemplateSolution (const Symbol & ref, const Symbol & solution) const;

	    /**
	     * ================================================================================
	     * ================================================================================
	     * =================================      SORTING    ==============================
	     * ================================================================================
	     * ================================================================================
	     */

	    std::vector <syntax::Expression> sort (const std::vector <syntax::Expression> & exprs, const std::map <std::string, syntax::Expression> & mapping) const;

	    std::vector <std::string> sortNames (const std::vector <syntax::Expression> & exprs, const std::map <std::string, syntax::Expression> & mapping) const;


	    /**
	     * ================================================================================
	     * ================================================================================
	     * ==============================    FINAL_VALIDATION    ==========================
	     * ================================================================================
	     * ================================================================================
	     */

	    std::map <std::string, syntax::Expression> validateLambdaProtos (const std::vector<syntax::Expression> & exprs, const std::map <std::string, syntax::Expression> & mapping) const;

	    syntax::Expression  retreiveFuncPtr (const syntax::Expression & elem, const std::vector<syntax::Expression> & exprs) const;

	    generator::Generator validateTypeOrEmpty (const syntax::Expression & type, const std::map <std::string, syntax::Expression> & mapping) const;

	    std::vector<generator::Generator> validateTypeOrEmptyMultiple (const syntax::Expression & type, const std::map <std::string, syntax::Expression> & mapping) const;

	    void finalValidation (const Symbol & sym, const std::vector<syntax::Expression> & exprs, const Mapper & mapping, const syntax::Expression & test) const;


	    /**
	     * ================================================================================
	     * ================================================================================
	     * ==============================    FINAL_VALIDATION    ==========================
	     * ================================================================================
	     * ================================================================================
	     */
	    
	    Ymir::Error::ErrorMsg partialResolutionNote (const lexing::Word & loc, const Mapper & mapper) const;

	};
	
    }

}
