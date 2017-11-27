#pragma once

#include <gc/gc_cpp.h>
#include <ymir/semantic/pack/Namespace.hh>
#include <ymir/utils/Array.hh>

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
    
    class IFrameProto : public gc {
    private:
	
	Namespace _space;
	std::string _name;
	Symbol _type;
	std::vector <::syntax::Var> _vars;
	std::vector <::syntax::Expression> _tmps;
	std::string _extern;
	
    public:

	IFrameProto (std::string, Namespace, Symbol, std::vector<syntax::Var>, std::vector<syntax::Expression>);

	std::string& name ();

	Namespace space ();

	Symbol& type ();

	std::vector <syntax::Var>& vars ();

	std::string& externName (); 
	
	bool equals (IFrameProto* scd);
		
    };

    typedef IFrameProto* FrameProto;
    
}
