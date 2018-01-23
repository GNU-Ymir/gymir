#pragma once

#include <ymir/syntax/Word.hh>
#include <ymir/utils/memory.hh>

namespace syntax {
    class IParamList;
    typedef IParamList* ParamList;

    class IExpression;
    typedef IExpression* Expression;

    class IVar;
    typedef IVar* Var;
}

namespace semantic {

    class IValue;
    typedef IValue * Value;

    class IInfoType;
    typedef IInfoType* InfoType;

    class ISymbol;
    typedef ISymbol* Symbol;
    
    class IValue  {
    public:
	
	virtual Value BinaryOp (Word token, Value right);

	virtual Value BinaryOpRight (Word token, Value left);

	virtual Value UnaryOp (Word token);

	virtual Value AccessOp (syntax::ParamList params); 
    
	virtual Value AccessOp (syntax::Expression expr);

	virtual Value CastOp (InfoType type);

	virtual Value CompOp (InfoType type);

	virtual Value CastTo (InfoType type);

	virtual Value DotOp (syntax::Var attr);

	virtual const char* getId () = 0;
		
	template <typename T>
	T* to () {
	    if (strcmp (this-> getId (), T::id ()) == 0) {
		return (T*) this;
	    }
	    return NULL;
	}

	template <typename T>
	bool is () {
	    return strcmp (this-> getId (), T::id ()) == 0;
	}
	
	virtual std::string toString ();

	virtual syntax::Expression toYmir (Symbol);

	virtual bool equals (Value) {
	    return false;
	}
	
	virtual Value clone () = 0;

	virtual ~IValue () {}
	
    };


    
}
