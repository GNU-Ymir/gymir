#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {


    /**
     * \struct IConstArray
     * The syntaxic node representation of a const array
     * \verbatim
     const_array := '[' (expression (',' expression)*)? ']' 
     \endverbatim
     */
    class IConstArray : public IExpression {
	
	/** The expressions contained in the const array*/
	std::vector <Expression> params;

	/** 
	 * Generated at semantic analyses 
	 * This array is used to cast the parameters in the right common type 
	 */
	std::vector <semantic::InfoType> casters;
	
    public:

	/**
	 * \param token the location of the array
	 * \param params the value contained inside the array
	 */
	IConstArray (Word token, const std::vector <Expression> &params);

	Expression expression () override;
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;	

	/**
	 * \return the number of parameters of the array
	 */
	int getNbParams ();

	/**
	 * \return the nb'th parameters
	 */
	Expression getParam (int nb);
	
	static const char * id () {
	    return TYPEID (IConstArray);
	}
	
	std::vector <std::string> getIds () override;

	Ymir::Tree toGeneric () override;

	std::string prettyPrint ();
	
	void print (int nb = 0) override;

	virtual ~IConstArray ();
	
    private:

	semantic::InfoType validate ();
	
    };
    
    typedef IConstArray* ConstArray;

}
