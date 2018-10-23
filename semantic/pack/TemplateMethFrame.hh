#pragma once

//#include "Frame.hh"
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
    
    class ITemplateMethFrame : public ITemplateFrame {
	
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

	ITemplateMethFrame (Namespace, std::string, InfoType, syntax::TypeMethod);

	FrameProto validate (ApplicationScore score, const std::vector<InfoType> & params);
	
	ApplicationScore isApplicable (syntax::ParamList params) override;

	ApplicationScore isApplicable (const std::vector <InfoType> & params) override;
	
	InfoType & getInfo ();
	
	
	static const char * id () {
	    return "ITemplateMethFrame";
	}
	
	virtual const char* getId ();	

	bool& isExtern ();
	
	bool & isVirtual ();

	bool& needConst ();
	
	syntax::TypeMethod getMethod ();	

    protected:
	
	Frame getScoreTempOp (const std::vector <syntax::Expression>& params) override;

	ApplicationScore getScoreVaridadic (Word ident, const std::vector<syntax::Var> & attrs, const std::vector<InfoType> & args) override;
    };

    typedef ITemplateMethFrame* TemplateMethFrame;
    
}
