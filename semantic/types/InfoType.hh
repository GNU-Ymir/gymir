#pragma once

#include <gc/gc_cpp.h>
#include "../../syntax/Word.hh"
#include "../pack/Frame.hh"
#include "../../errors/Error.hh"
#include <vector>

namespace semantic {


    // typedef Tree (InstComp*)(Tree, Tree);
    // typedef Tree (InstCompMult*)(Tree, std::vector <Tree>);
    // typedef Tree (InstCompS*)(Tree);
    // typedef Tree (InstPreTreatment*)(InfoType, Expression, Expression);
    
    class IInfoType;
    typedef IInfoType* InfoType;
    
    class IApplicationScore : public gc {
    public:

	ApplicationScore ();
	ApplicationScore (Word, bool);

	long score;
	Word token;
	std::string name;
	bool dyn;
	InfoType left;
	InfoType ret;
	std::vector <InfoType> treat;
	std::map <std::string, Expression> tmps;
	bool isVariadic;
	bool isTemplate;
	Frame toValidate;
    };

    typedef IApplicationScore* ApplicationScore;

    class IInfoType : public gc {

	bool _isConst = false;
	bool _isStatic = false;
	ulong _toGet;

	//Value _value;

	static std::map<std::string, IInfoType*> alias;
	
    protected:

	bool _isType = false;


    public:

	IInfoType (bool isConst);

	static InfoType factory (Word word, std::vector <Expression> templates) {
	    auto it = Creators::instance ().find (word.getStr ());
	    if (it != NULL) {
		return (*it) (word, templates);
	    }
	    auto _it_ = this-> alias.find (word.getStr ());
	    if (_it_ != NULL) return _it_-> clone ();
	    Ymir::Error::append ("Type inconnu '%s%s%s'",
				 Ymir::Error::YELLOW,
				 word.getStr ().c_str (),
				 Ymir::Error::RESET);
	}
	
    };    

}
