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
    
    class IDestructFrame : public IFrame {
	
	semantic::InfoType _info;

	syntax::TypeDestructor _dest;	
	std::string _name;
	bool _isExtern = false;

	bool _isInnerPrivate = false;
	bool _isInnerProtected = false;

	bool _echec = false;
	bool _needConst = false;
	
	FrameProto _proto;
	
    public:
	
	IDestructFrame (Namespace, std::string, InfoType, syntax::TypeDestructor);	       

	FrameProto validate (const std::vector <InfoType> & params) override;

	FrameProto validate (syntax::ParamList params) override;

	FrameProto validate () override;

	ApplicationScore isApplicable (syntax::ParamList params) override;

	ApplicationScore isApplicable (const std::vector <InfoType> & params) override;
	
	InfoType & getInfo ();	
	
	static const char * id () {
	    return "IDestructFrame";
	}
	
	virtual const char* getId ();	

	bool& isExtern ();
	
	bool& needConst ();


    private :

	
    };

    typedef IDestructFrame* DestructFrame;
    
}
