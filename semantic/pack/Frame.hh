#pragma once

#include <gc/gc_cpp.h>
#include "Namespace.hh"
#include "FrameProto.hh"
#include "FrameScope.hh"

#include <string>

namespace semantic {


    namespace syntax {
	class IExpression;
	typedef IExpression* Expression;

	class IParamList;
	typedef IParamList* ParamList;

	class IFunction;
	typedef IFunction* Function;
    }
    
    class IFrame : public gc {
    protected:
	
	syntax::Function _function;
	Namespace _space;
	std::string _imutSpace;

	static const long CONST_SAME;// = 18;
	static const long SAME;// = 20;
	static const long CONST_AFF;// = 8;    
	static const long AFF;// = 10;
	static const long CHANGE;// = 14;			       

	int _currentScore;        
	bool _isInternal = false;	
	bool _isPrivate = false;
	bool _isVariadic = false;

	std::vector <syntax::Expression> tempParams;

    public:

	IFrame (Namespace space, syntax::Function func);
	    
	virtual FrameProto validate ();

	virtual FrameProto validate (syntax::ParamList);

	virtual Namespace& space ();

	virtual int& currentScore ();

	virtual bool& isInternal ();

	virtual bool isVariadic () const;

	virtual void isVariadic (bool isVariadic);

	static void verifyReturn (Word token, Symbol ret, FrameReturnInfo infos);

	//virtual ApplicationScore isApplicable (ParamList params);

	syntax::Function func ();

	virtual Word ident ();
	
	
    };

    typedef IFrame* Frame;
    
}
