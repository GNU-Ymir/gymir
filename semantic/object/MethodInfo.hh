#pragma once

#include <ymir/semantic/types/FunctionInfo.hh>
#include <ymir/semantic/object/AggregateInfo.hh>

namespace semantic {

    class IMethodInfo : public IInfoType {

	AggregateInfo _info;
	std::string _name;
	std::vector <Frame> _frames;
	std::vector <int> _index;

	
    public :

	IMethodInfo (AggregateInfo info, std::string name, const std::vector <Frame> & frames, const std::vector <int> & index);

	bool isSame (InfoType) override;

	InfoType onClone () override;

	std::vector <Frame> getFrames ();

	ApplicationScore CallOp (Word, syntax::ParamList) override;
		
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	static const char * id () {
	    return "IMethodInfo";
	}

	const char* getId () override;	

    private :

	ApplicationScore CallAndThrow (Word tok, const std::vector <InfoType> & params, InfoType & ret);
	
    };

    typedef IMethodInfo* MethodInfo;
    
}
