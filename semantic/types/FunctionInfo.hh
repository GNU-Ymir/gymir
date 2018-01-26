#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/pack/Namespace.hh>
#include <vector>
#include <map>
#include <ymir/utils/Array.hh>

namespace semantic {

    class IFunctionInfo : public IInfoType {

	std::string _name;
	Namespace _space;
	Frame _info;
	std::vector <Frame> _fromTemplates;
	bool _alone = false;

    public:

	IFunctionInfo (Namespace, std::string);

	IFunctionInfo (Namespace, std::string, const std::vector<Frame> & infos);

	bool isSame (InfoType) override;

	void set (Frame);

	Frame frame ();

	Namespace space ();

	InfoType onClone () override;

	std::vector <Frame> getFrames ();

	ApplicationScore CallOp (Word, const std::vector<InfoType> &) override;

	ApplicationScore CallOp (Word, syntax::ParamList) override;

	InfoType UnaryOp (Word) override;

	InfoType TempOp (const std::vector<syntax::Expression> &);

	std::string innerTypeString () override;

	std::string typeString () override;

	std::string innerSimpleTypeString () override;

	bool& alone ();
	
	std::string name ();

	static const char* id () {
	    return "IFunctionInfo";
	}

	const char* getId () override;
	
    private:

	InfoType toPtr ();

	std::map<Word, std::string> candidates ();
	
    };

    typedef IFunctionInfo* FunctionInfo;
    
}
