#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IPragma : public IExpression {

	ParamList params;
	
	static const std::string COMPILE;// = "compile";
	static const std::string MSG;// = "msg";

    public :

	IPragma (Word token, ParamList params);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Ymir::Tree toGeneric () override;

	std::string prettyPrint () override;
	
	static const char * id () {
	    return TYPEID (IPragma);
	}
	
	virtual std::vector <std::string> getIds () override {
	    auto ids = IInstruction::getIds ();
	    ids.push_back (TYPEID (IPragma));
	    return ids;
	}

	
    private :

	void executeMsg ();

	Expression executeCompile ();       
	
    };

    typedef IPragma* Pragma;
    
}
