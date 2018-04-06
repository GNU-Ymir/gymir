#pragma once

#include <ymir/utils/memory.hh>
#include <ymir/semantic/pack/Namespace.hh>
#include <ymir/utils/Array.hh>
#include <ymir/semantic/tree/Tree.hh>

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;

    class IParamList;
    typedef IParamList* ParamList;

    class IFunction;
    typedef IFunction* Function;

    class IVar;
    typedef IVar* Var;    
}


namespace semantic {

    class IInfoType;
    typedef IInfoType* InfoType;

    class ISymbol;
    typedef ISymbol* Symbol;

    class IFinalFrame;
    typedef IFinalFrame* FinalFrame;
    
    class IFrameProto  {
    private:
	
	Namespace _space;
	std::string _name;
	Symbol _type;
	std::vector <::syntax::Var> _vars;	
	std::vector <::syntax::Var> _closure;
	std::vector <::syntax::Expression> _tmps;
	std::vector <Word> _attributes;
	std::string _extern;
	Ymir::Tree _fn;
	bool _isCVariadic;
	bool _isForced = false;
	FinalFrame _attached;
	bool _isLvalue = false;
	bool _isMoved = false;
	
    public:

	IFrameProto (std::string, Namespace, Symbol, const std::vector<syntax::Var>&, const std::vector<syntax::Expression>&, std::vector <Word> attrs);

	std::string& name ();

	Namespace space ();

	Symbol& type ();

	std::vector <syntax::Var>& vars ();

	std::vector <syntax::Var>& closure ();

	std::vector <syntax::Expression>& tmps ();

	std::vector <Word> & attributes ();
	
	std::string& externName (); 
	
	bool isDelegate ();

	bool equals (IFrameProto* scd);

	bool& isCVariadic ();
	
	FinalFrame & attached ();

	bool& isLvalue ();

	bool& isMoved ();
	
	bool has (std::string attrs);
	
	void isForcedDelegate ();

	Ymir::Tree toGeneric ();

	Ymir::Tree createClosureType ();
	
    };

    typedef IFrameProto* FrameProto;
    
}
