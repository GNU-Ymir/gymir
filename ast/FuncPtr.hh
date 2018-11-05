#pragma once

#include "Expression.hh"
#include <vector>

#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    /**
     * \struct IFuncPtr
     * The syntaxic node representation of a function pointer
     * \verbatim
     func_ptr := 'fn' '(' (type (',' type)*)? ')' '->' type
     \endverbatim
     */
    class IFuncPtr : public IExpression {

	/** The type parameters */
	std::vector <Expression> _params;

	/** The return type */
	Expression _ret;

	
    public:

	/**
	 * \param begin the location of the expression
	 * \param params the parameters types 
	 * \param type the return type 
	 */
	IFuncPtr (Word begin, std::vector <Expression> params, Expression type);

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Expression expression () override;

	/**
	 * \return get or set the list of parameters types
	 */
	std::vector <Expression> & params ();
	
	/**
	 * \return the list of parameters types
	 */
	const std::vector <Expression> & getParams ();

	/**
	 * \return the return expression
	 */
	Expression getRet ();
	
	Ymir::Tree toGeneric () override;
	
	static const char * id ();

	std::vector <std::string> getIds () override;

	std::string prettyPrint () override;
	
	void print (int nb = 0) override;
	
	virtual ~IFuncPtr ();
	
    };

    typedef IFuncPtr* FuncPtr;

}
