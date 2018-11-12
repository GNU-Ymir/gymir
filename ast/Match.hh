#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>
#include <ymir/semantic/pack/DestructSolver.hh>

namespace semantic {
    struct DestructSolution;
}

namespace syntax {

    /** 
     * \struct IMatch
     * The syntaxic node representation of a match
     * \verbatim
     match := 'match' expression '{' (match_expression '=>' block)+ '}'
     \endverbatim
     */
    class IMatch : public IExpression {
	
	/** The expression that will be used for pattern matching */
	Expression _expr;

	/** The different values to test */
	std::vector<Expression> _values;

	/** The block associated to the values */
	std::vector<Block> _block;

	/** The casters expression (produced at semantic) */
	std::vector <semantic::InfoType> _casters;

	/** The different solutions */
	std::vector <semantic::DestructSolution> _soluce;

	/** 
	 * Expression produced at semantic time 
	 * This one is used to get a reference to the real expression, and evaluate it only one time 
	 * when there are multiple values in the match
	 * \verbatim
	 match (foo ()) { // The foo function will be called only one time, and value store into '_' (this-> _aux)
	     (1, 2) => println (12);
	     (2, 3) => println (23); 
	 }
	 \endverbatim
	 */	
	Expression _aux;

	/** The affectation of the auxiliary var */
	Expression _binAux;
	
    public:

	/**
	 * \param word the location of the expression
	 * \param expr the expression that will be used for matching 
	 */
	IMatch (Word word, Expression expr);

	/**
	 * \param word the location of the expression
	 * \param expr the expression that will be used for matching 
	 * \param values the list of values (patterns)
	 * \param block the list of block associated to the values
	 */
	IMatch (Word word, Expression expr, const std::vector<Expression> &values, const std::vector <Block> &block);

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	std::vector <semantic::Symbol> allInnerDecls () override;
	
	Expression expression () override;

	Instruction instruction () override;
	
	Ymir::Tree toGeneric () override;
	
	static const char * id () {
	    return TYPEID (IMatch);
	}
	
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IMatch));
	    return ret;
	}
	
	void print (int nb = 0) override;

	virtual ~IMatch ();

    private:
	
	Ymir::Tree declareAndAffectAux ();
	
	Ymir::Tree declareVars (std::vector <Var> vars, std::vector <Expression> caster);	

	Ymir::Tree validateBlock (Expression test, Block bl, Ymir::Tree elsePart, Ymir::Tree affectPart);

	Ymir::Tree validateBlockExpr (Expression test, Block bl, semantic::InfoType caster, Ymir::Tree res, Ymir::Tree elsePart, Ymir::Tree affectPart);

	Ymir::Tree toGenericExpression (Ymir::TreeStmtList list, Ymir::Tree);
	
	semantic::InfoType validate (std::vector <semantic::Symbol> & syms);	

    };

    typedef IMatch* Match;

}
