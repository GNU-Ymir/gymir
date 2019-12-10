#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Catch
     * A scope is a event that append at the end of a scope 
     * It can be a failure, a success or just a standard exit
     * \verbatim
     scope := 'on' Identifier '=>' expression
     \endverbatim
     */
    class Catch : public IExpression {
    protected:
	
	std::vector <Expression> _vars;

	std::vector <Expression> _actions;

    protected :

	friend Expression;

	Catch ();
	
	Catch (const lexing::Word & loc, const std::vector<Expression> & vars, const std::vector<Expression> & actions);

    public :

	/**
	 * \brief Create a new Catch 
	 * \param location the location of the scope
	 * \param content the content of the scope
	 */
	static Expression init (const lexing::Word & location, const std::vector<Expression> & vars, const std::vector<Expression> & actions);

	virtual Expression clone () const override;

	virtual bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
	const std::vector <Expression> & getVars () const;

	const std::vector <Expression> & getActions () const;
	
    };    
    
}
