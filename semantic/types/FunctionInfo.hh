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
	std::vector <Frame> alls;
	bool _alone = false;

	static int nbTmpsCreation;// = 0;
	static bool needToReset;// = true;
	bool itsUpToMe = false;
	
    public:

	IFunctionInfo (Namespace, std::string);

	IFunctionInfo (Namespace, std::string, const std::vector<Frame> & infos);

	bool isSame (InfoType) override;

	bool passingConst (InfoType) override;
	
	void set (Frame);

	Frame frame ();

	Namespace space ();

	InfoType onClone () override;

	InfoType BinaryOpRight (Word op, syntax::Expression left) override;
	
	std::vector <Frame> getFrames ();

	ApplicationScore CallOp (Word, syntax::ParamList) override;

	InfoType UnaryOp (Word) override;

	InfoType TempOp (const std::vector<syntax::Expression> &);

	InfoType CompOp (InfoType other) override;
	
	std::string innerTypeString () override;

	std::string typeString () override;

	std::string innerSimpleTypeString () override;

	InfoType getTemplate (ulong i) override;

	std::vector <InfoType> getTemplate (ulong i, ulong af) override;

	ulong nbTemplates () override;
	
	bool& alone ();
	
	std::string name ();

	static const char* id () {
	    return "IFunctionInfo";
	}

	const char* getId () override;
	
    private:

	InfoType toPtr ();

	std::map<Word, std::string> candidates ();

	ApplicationScore verifErrors ();

	ApplicationScore CallAndThrow (Word tok, const std::vector <InfoType>& params, FrameProto & info);

	ApplicationScore CallOp (Word, const std::vector<InfoType> &);
	
    };

    typedef IFunctionInfo* FunctionInfo;
    
}
