#pragma once

#include "Expression.hh"
#include "ParamList.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <vector>

namespace syntax {


    /**
     * \struct IAccess
     * Syntax node of access operator : \n
     * \verbatim 
     access := expression '[' param_list ']' 
     \endverbatim
     */
    class IAccess : public IExpression {
	/// The closure token \a ]
	Word _end;

	/// The set of parameters used 
	ParamList _params;

	/// The expression on the left side <em>left [...]</em>
	Expression _left;

	/// The mandatories treatments to execute on the parameters when using the operator
	std::vector <semantic::InfoType> _treats;	
	
    public:

	/**
	 * \param word  the location of the token \a [
	 * \param end  the location of the token \a ]
	 * \param left the expression of the left side of this expression
	 * \param params the list of parameters between \a [ and \a ]
	 */
	IAccess (Word word, Word end, Expression left, ParamList params);

	/**
	 * \brief Empty access construction 
	 * \param word the location of the token \a [
	 * \param end the location of the token \a ]
	 */
	IAccess (Word word, Word end);

	/**
	 * \brief Perform the semantic analyses 
	 * \return a typed expression or NULL, if the analyse failed
	 */
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>& values) override;

	/**
	 * \return the left operand of the expression
	 */
	Expression getLeft ();


	/**
	 * \return the list of parameters of the expression
	 */
	std::vector <Expression> getParams ();


	/**
	 * \brief Generate a valid GCC tree used for code generation
	 * \return the valid tree, or Tree () if failure append	 
	 */
	Ymir::Tree toGeneric () override;

	/**
	 * \return true
	 */
	bool isLvalue () override;

	/**
	 * \deprecated Only used for debugging purpose
	 * \brief print the AST of this expression
	 * \param nb the current left alignement
	 */
	void print (int nb = 0) override;

	/**
	 * \brief Generate a string representing the source code of this expression
	 * \return a string
	 */
	std::string prettyPrint () override;

	/**
	 * \brief Type information for smart casting 
	 */
	static const char * id () {
	    return TYPEID (IAccess);
	}

	/**
	 * \return [Expression::getIds (), IAccess::id ()]
	 */
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IAccess));
	    return ret;
	}

	virtual ~IAccess ();
	
    private:

	/**
	 * \brief Search an valid opAccess frame in order to override the operator	 
	 */
	Expression findOpAccess ();
	
    };

    typedef IAccess* Access;

}
