#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IApplicationScore;
    typedef IApplicationScore* ApplicationScore;
}

namespace syntax {
    
    class IDotCall;
    class IParamList;
    typedef IParamList* ParamList;

    /**
     * \struct IPar
     * The syntaxic node representation of a parentheses operator 
     * \verbatim
     par := expression '(' param_list ')'
     \endverbatim
     */
    class IPar : public IExpression {

	/** The closure token */
	Word _end;

	/** The parameters */
	ParamList _params;

	/** The left operand */
	Expression _left;

	/** In the case of semantic transformation to dotCall */
	IDotCall* _dotCall;

	/** Used an override */
	bool _opCall = false;

	/** The score of the call resolution (containing all the needed information to transform into lint language) */
	semantic::ApplicationScore _score;
	
    public :

	/** 
	 * \param word the location of the operator
	 * \param end the closure token of the operator
	 */
	IPar (Word word, Word end);

	/** 
	 * \param word the location of the operator
	 * \param end the closure token of the operator
	 * \param left the left operand
	 * \param params the parameters 
	 * \param fromOpCall created at semantic time to override an OpCall ?
	 */
	IPar (Word word, Word end, Expression left, ParamList params, bool fromOpCall = false);

	/**
	 * \return get or set the parameters 
	 */
	ParamList& paramList ();	

	/**
	 * \return get or set the left operand 
	 */
	Expression& left ();

	/**
	 * \return get or set the score of the call resolution
	 */
	semantic::ApplicationScore& score ();

	/**
	 * \return get or set the dotCall
	 */
	IDotCall*& dotCall (); 

	/**
	 * \return true, iif the call resolution is a lvalue (return type is reference ...)
	 */
	bool isLvalue () override;
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;

	static const char * id () {
	    return TYPEID (IPar);
	}

	std::string prettyPrint () override;
	
	std::vector <std::string> getIds () {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID(IPar));
	    return ret;
	}
	
	void print (int nb = 0) override;

	virtual ~IPar ();
	
    private:

	/**
	 * \brief common verification 
	 */
	bool simpleVerif (IPar*&);

	void tuplingParams (semantic::ApplicationScore score, IPar* par);

	Ymir::Tree createClosureVar ();

	Ymir::Tree callInline (std::vector <tree> args);

	Expression findOpCall ();
	
    };

    typedef IPar* Par;    
}
