#pragma once

#include "Expression.hh"
#include <vector>
#include "../errors/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    /**
     * \struct IParamList
     * The syntaxic node representation of a paramlist
     * \verbatim
     param_list := (expression (',' expression)*)?
     \endverbatim
     */
    class IParamList : public IExpression {

	/** No need to document */
	std::vector <Expression> _params;

	/** The treatments to apply to the different parameters at lint time */
	std::vector <semantic::InfoType> _treats;
	
    public :

	/**
	 * \param ident the location of the list
	 * \param params the parameters
	 */
	IParamList (Word ident, const std::vector <Expression>& params);

	/**
	 * \return get or set the list of parameters 
	 */
	std::vector <Expression>& params ();
	
	/**
	 * \return the list of parameters 
	 */
	const std::vector <Expression>& getParams ();

	/**
	 * \brief construct a new list containaing the types of all the parameters
	 * \warning expression () must be called before, to perform the semantic analyse, otherwise it will crash
	 * \return the list of the types of the parameters (references)
	 */
	std::vector <semantic::InfoType> getParamTypes ();

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	/**
	 * \brief transform the list of parameters to a generic gimple list 
	 */
	std::vector <tree> toGenericParams (const std::vector <semantic::InfoType>&);
	
	/**
	 * \return get or set the cast treatments of the parameters for lint time
	 */
	std::vector <semantic::InfoType> & treats ();

	/**
	 * \return the cast treatments of the parameters for lint time
	 */
	const std::vector <semantic::InfoType> & getTreats ();

	std::string prettyPrint () override;
	
	void print (int nb = 0) override;

	static const char * id () {
	    return TYPEID (IParamList);
	}
	
	std::vector <std::string> getIds () override;

	virtual ~IParamList ();
	
    };

    typedef IParamList* ParamList;
    
}
