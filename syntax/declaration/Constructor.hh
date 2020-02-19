#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/declaration/Function.hh>
#include <ymir/syntax/Decorator.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    /**
     * \struct Constructor
     * Declaration of a class Constructor
     * \verbatim
     constructor := 'self templates '(' (var_decl (',' var_decl)*)? ')' ('with' ('super' '(' (expression (',' expression)*)? ')' )? (',' Identifier '=' expression)* body 
     \endverbatim
     */
    class Constructor : public IDeclaration {
    private:

	lexing::Word _name;

	Function::Prototype _proto;

	std::vector <Expression> _superParams;

	std::vector <std::pair <lexing::Word, Expression> > _construction;

	Expression _body;

	lexing::Word _explicitSuperCall;

	lexing::Word _explicitSelfCall;

	/** The CAs (custom attributes) of the functions */
	std::vector <lexing::Word> _cas;

	std::vector <Expression> _throwers;
	
    private : 

	friend Declaration;

	Constructor ();

	Constructor (const lexing::Word & name, const Function::Prototype & proto, const std::vector <Expression> & super, const std::vector <std::pair <lexing::Word, Expression> > & constructions, const Expression & body, const lexing::Word & explicitSuperCall, const lexing::Word & explicitSelfCall, const std::vector <lexing::Word> & cas, const std::vector <Expression> & thrower);

    public :

	/**
	 * \brief Create a new constructor 
	 * \param name the name of the constructor (its location)
	 * \param proto the prototype of the constructor
	 * \param super the constructor params for the super class
	 * \param constructions the constructions default value of the fields
	 * \param body the body of the constructor
	 */
	static Declaration init (const lexing::Word & name, const Function::Prototype & proto, const std::vector <Expression> & super, const std::vector <std::pair <lexing::Word, Expression> > & constructions, const Expression & body, const lexing::Word & explicitSuperCall, const lexing::Word & explicitSelfCall, const std::vector <lexing::Word> & cas, const std::vector <Expression> & thrower);

	/**
	 * Mandatory function used for proxy polymoprhism system
	 */
	Declaration clone () const override;

	/**
	 * \brief Polymorphism dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;

	/**
	 * \return the location of the constructor
	 */
	const lexing::Word & getName () const;

	/**
	 * \return the list of parameters to pass for the construction of the super class
	 */	
	const std::vector <Expression> & getSuperParams () const;

	/**
	 * \return the list of affectation for field constructions
	 */
	const std::vector <std::pair <lexing::Word, Expression> > & getFieldConstruction () const;
	
	/**
	 * \return the prototype of the constructor 
	 * \warning constructor prototypes always have empty return type
	 */
	const Function::Prototype & getPrototype () const;

	/**
	 * \return the body of the constructor
	 */
	const Expression & getBody () const;

	/**
	 * \return the location of the explicit super call
	 * \warning might be eof, if the super is not called
	 */
	const lexing::Word & getExplicitSuperCall () const;

	/**
	 * \return the location of the explicit self call
	 * \warning might be eof, if the not other constructor is called
	 */
	const lexing::Word & getExplicitSelfCall () const;

	/**
	 * \return the list of custom attributs
	 */
	const std::vector <lexing::Word> & getCustomAttributes () const;


	const std::vector <syntax::Expression> & getThrowers () const;

    };

    

}
