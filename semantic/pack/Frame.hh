#pragma once

#include <gc/gc_cpp.h>
#include "Namespace.hh"
#include "FrameProto.hh"
#include "FrameScope.hh"

#include <string>
#include <ymir/utils/Array.hh>

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;

    class IParamList;
    typedef IParamList* ParamList;

    class IFunction;
    typedef IFunction* Function;
}

namespace semantic {

    class IApplicationScore;
    typedef IApplicationScore* ApplicationScore;
    
    class IInfoType;
    typedef IInfoType* InfoType;
    
    class IFrame : public gc {
    protected:
	
	::syntax::Function _function;
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

	std::vector <::syntax::Expression> tempParams;

    public:

	IFrame (Namespace space, ::syntax::Function func);
	    
	virtual FrameProto validate ();

	virtual FrameProto validate (std::vector <InfoType>);
	
	virtual FrameProto validate (std::vector <::syntax::Var>);
	
	virtual FrameProto validate (::syntax::ParamList);

	virtual FrameProto validate (ApplicationScore, std::vector <InfoType>);

	static std::vector <::syntax::Var> computeParams (std::vector<::syntax::Var>, std::vector<InfoType>);

	static std::vector <::syntax::Var> computeParams (std::vector<::syntax::Var>);

	virtual Namespace& space ();

	virtual int& currentScore ();

	virtual bool& isInternal ();

	virtual bool isVariadic () const;

	virtual void isVariadic (bool isVariadic);

	static void verifyReturn (Word token, Symbol ret, FrameReturnInfo infos);

	virtual ApplicationScore isApplicable (syntax::ParamList params);

	virtual ApplicationScore isApplicable (std::vector <InfoType> params);

	::syntax::Function func ();

	virtual Word ident ();

	template <typename T>
	bool is () {
	    return strcmp (this-> getId (), T::id ()) == 0;
	}
	
	template <typename T>
	T* to () {
	    if (strcmp (this-> getId (), T::id ()) == 0) {
		return (T*) this;
	    } else return NULL;
	}	

	static const char* id () {
	    return "IFrame";
	}

	virtual const char* getId () = 0;

    protected:

	ApplicationScore getScore (Word ident, std::vector <syntax::Var>, std::vector <InfoType>);

	ApplicationScore isApplicable (Word ident, std::vector <syntax::Var>, std::vector <InfoType>);
	
    };

    typedef IFrame* Frame;
    
}
