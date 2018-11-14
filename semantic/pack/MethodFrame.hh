#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include "UnPureFrame.hh"
#include "PureFrame.hh"
#include "TemplateFrame.hh"
#include <ymir/ast/ParamList.hh>

namespace syntax {
    class ITypeConstructor;
    typedef ITypeConstructor* TypeConstructor;

    class ITypeDestructor;
    typedef ITypeDestructor* TypeDestructor;

    class ITypeMethod;
    typedef ITypeMethod* TypeMethod;
    
    class IParamList;
    typedef IParamList* ParamList; 

    class IExpression;
    typedef IExpression* Expression;
    
}


namespace semantic {
    
    class IMethodFrame : public IFrame {
	
	semantic::InfoType _info;
	syntax::TypeMethod _method;
	
	std::string _name;
	bool _isExtern = false;
	bool _isVirtual = true;
	bool _isInnerPrivate = false;
	bool _isInnerProtected = false;
	bool _echec = false;
	bool _needConst = false;
	FrameProto _proto;
	
    public:

	IMethodFrame (Namespace, std::string, InfoType, syntax::TypeMethod);
	

	FrameProto validate (const std::vector <InfoType> & params) override;

	FrameProto validate (syntax::ParamList params) override;

	FrameProto validate () override;

	ApplicationScore isApplicable (syntax::ParamList params) override;

	ApplicationScore isApplicable (const std::vector <InfoType> & params) override;
	
	InfoType & getInfo ();
	
	
	static const char * id () {
	    return "IMethodFrame";
	}
	
	virtual const char* getId ();	

	bool isExtern ();

	void isExtern (bool);
	
	bool isVirtual ();

	void isVirtual (bool);

	bool& needConst ();
	
	syntax::TypeMethod getMethod ();	

    private :
 	
    };

    typedef IMethodFrame* MethodFrame;
    
}
