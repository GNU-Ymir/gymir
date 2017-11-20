#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/pack/Namespace.hh>
#include <vector>
#include <map>

namespace semantic {

    class IFunctionInfo : public IInfoType {

	std::string _name;
	Namespace _space;
	Frame _info;
	std::vector <Frame> _fromTemplates;
	bool _alone = false;

    public:

	IFunctionInfo (Namespace, std::string);

	IFunctionInfo (Namespace, std::string, std::vector<Frame> infos);

	bool isSame (InfoType) override;

	void set (Frame);

	Frame frame ();

	Namespace space ();

	InfoType clone () override;

	std::vector <Frame> getFrames ();

	ApplicationScore CallOp (Word, std::vector <InfoType>) override;

	ApplicationScore CallOp (Word, syntax::ParamList) override;

	InfoType UnaryOp (Word) override;

	InfoType TempOp (std::vector<syntax::Expression>);

	std::string innerTypeString () override;

	std::string simpleTypeString () override;

	bool& alone ();

	std::string name ();

	static const char* id () {
	    return "IFloatInfo";
	}

	const char* getId () override;
	
    private:

	InfoType toPtr ();

	std::map<Word, std::string> candidates ();
	
    };

    typedef IFunctionInfo* FunctionInfo;
    
}
