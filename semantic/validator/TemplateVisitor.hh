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
		
		uint size () const {
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
	    
	    struct Mapper {
		bool succeed;
		uint score;
		std::map <std::string, syntax::Expression> mapping;
		std::vector <std::string> nameOrder;

		Mapper (bool succ, uint score) {
		    this-> succeed = succ;
		    this-> score = score;
		}

		Mapper (bool succ, uint score, const std::map <std::string, syntax::Expression> & mapping, const std::vector <std::string> & names) {
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
	     * \brief Validate a template from template call explicit specialization
	     * \param params the parameter of the template
	     * \param valls the parameter pass to the call
	     */
	    Mapper validateFromExplicit (const std::vector <syntax::Expression> & params, const std::vector <generator::Generator> & calls) const;
	    
	    /**
	     * \brief Validate a template from template call explicit specialization
	     * \param ref the template reference used in the call
	     * \param params the parameters passed at the call
	     */
	    semantic::Symbol validateFromExplicit (const generator::TemplateRef & ref, const std::vector <generator::Generator> & params, int & score) const;	    


	    Mapper validateParamTemplFromExplicit (const array_view <syntax::Expression> & paramTempl, const syntax::Expression & param, const array_view <generator::Generator> & values, int & consumed) const;
	    
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
	     * \return a Mapper, which stores the modifications to apply
	     */
	    Mapper validateTypeFromImplicit (const array_view <syntax::Expression> & params, const syntax::Expression & left, const array_view <generator::Generator> & types, int & consumed) const;


	    /**
	     * \brief Validate a type template specialization from implicit context
	     * \param params the template parameters
	     * \param cl a template call from a vardecl, (a : X!(T))
	     * \param type the type associated to this vardecl for implicit specialization
	     * \return a Mapper, which store the modification to apply
	     */
	    Mapper validateTypeFromTemplCall (const array_view <syntax::Expression> & params, const syntax::TemplateCall & cl, const generator::Generator & type, int & consumed) const;

	    
	    /**
	     * \brief Validate a type template specialization from implicit context
	     * \param params the template parameters
	     * \param cl a template call from a vardecl, (a : X!(T))
	     * \param the symbol of the template solution 
	     * \return a Mapper, which store the modification to apply
	     */
	    Mapper validateTypeFromTemplCall (const array_view <syntax::Expression> & params, const syntax::TemplateCall & cl, const semantic::TemplateSolution & tmplS, int & consumed) const;

	    /**
	     * \brief Validate a type template specialization from explicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param left one of the template parameters \in params
	     * \param type the type that will make the specialization
	     */
	    Mapper applyTypeFromExplicit (const array_view <syntax::Expression> & params, const syntax::Expression & left, const array_view <generator::Generator> & types, int & consumed) const;

	    /**
	     * \brief Validate a type template specialization from explicit context on an OfVar
	     * \param params the template parameters (T of [R], R, ...)
	     * \param left one of the template parameters \in params, which is a OfVar
	     * \param type the type that will make the specialization
	     */
	    Mapper applyTypeFromExplicitOfVar (const array_view <syntax::Expression> & params, const syntax::OfVar & var, const generator::Generator & types) const;

	    /**
	     * \brief Validate a type template specialization from explicit context on an ImplVar
	     * \param params the template parameters (T impl isTruc!R, R, ...)
	     * \param left one of the template parameters \in params, which is a ImplVar
	     * \param type the type that will make the specialization
	     */
	    Mapper applyTypeFromExplicitImplVar (const array_view <syntax::Expression> & params, const syntax::ImplVar & implvar, const generator::Generator & types) const;


	    /**
	     * \brief Validate a decorated expression specialization from implicit context
	     * \param params the template parameters (T : [R], R, ...)
	     * \param expr the decorated expression 
	     */
	    Mapper applyTypeFromDecoratedExpression (const array_view <syntax::Expression> & params, const syntax::DecoratedExpression & expr, const array_view <generator::Generator> & types, int & consumed) const; 
	    
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
	     * \brief Create syntax tree from generator type
	     * \brief Reverse the compilation (kind of, it just return a SyntaxWrapper for a generator)
	     */
	    syntax::Expression createSyntaxType (const lexing::Word & loc, const generator::Generator & type, bool isMutable = false, bool isRef = false) const;
	    
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
	     * \brief Return the first templatesolution in a tree of symbol (the closest to the root)
	     * \param symbol the symbol to test
	     */
	    semantic::Symbol getFirstTemplateSolution (const semantic::Symbol & symbol) const;

	    std::vector <syntax::Expression> sort (const std::vector <syntax::Expression> & exprs, const std::map <std::string, syntax::Expression> & mapping) const;

	    std::vector <std::string> sortNames (const std::vector <syntax::Expression> & exprs, const std::map <std::string, syntax::Expression> & mapping) const;

	    std::map <std::string, syntax::Expression> validateLambdaProtos (const std::vector<syntax::Expression> & exprs, const std::map <std::string, syntax::Expression> & mapping) const;

	    syntax::Expression  retreiveFuncPtr (const syntax::Expression & elem, const std::vector<syntax::Expression> & exprs) const;

	    generator::Generator validateTypeOrEmpty (const syntax::Expression & type, const std::map <std::string, syntax::Expression> & mapping) const;

	    void finalValidation (const Symbol & sym, const std::vector<syntax::Expression> & exprs, const Mapper & mapping, const syntax::Expression & test) const;

	    Symbol getTemplateSolution (const Symbol & ref, const Symbol & solution) const;
	    
	    std::string partialResolutionNote (const lexing::Word & loc, const Mapper & mapper) const;

	};
	
    }

}
