#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}


namespace syntax {

    
    /**
     * \struct IAffectGeneric 
     * Forced affectation, of potentially different type
     * \warning This class must be instantiated at semantic time only     
     */
    class IAffectGeneric : public IExpression {
	Expression _left, _right;
	bool _addr;
	
    public:

	/**
	 * \param word the location of the affectation
	 * \param left the left operand
	 * \param right the right operand
	 * \param addr is the affectation by reference ? \verbatim left = &right \endverbatim 
	 */
	IAffectGeneric (Word word, Expression left, Expression right, bool addr = false);

	/**
	 * \brief Perform a semantic analyse of the two operand
	 * \return an expression typed \a void
	 */
	Expression expression () override;

	IExpression* templateExpReplace (const std::map <std::string, Expression>&) override {
	    return this;    
	}
	
	Ymir::Tree toGeneric () override;
	
	void print (int) override {};
	
    };

    /**
     * \struct IFakeDecl
     * Declaration of local variable 
     * \warning This class must be instantiated at semantic time only
     */
    class IFakeDecl : public IExpression {
	Var _left;
	Expression _right;
	bool _addr, _const;
	
    public:

	/**
	 * \param word the location of the declaration
	 * \param left the variable to declare
	 * \param right the value to affect to the variable
	 * \param isConst is the variable constant
	 * \param addr is the affectation by reference
	 */
	IFakeDecl (Word word, Var left, Expression right, bool isConst, bool addr = false);

	Expression expression () override;

	IExpression* templateExpReplace (const std::map <std::string, Expression>&) override {
	    return this;    
	}
		
	void print (int) override {};
	
    };

    

    typedef IFakeDecl* FakeDecl;
    typedef IAffectGeneric* AffectGeneric;
  
}


