#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IPragma
     * Pragma operation 
     * \verbatim
     pragma := '__pragma' '(' prg_type ',' param_list ')'
     prg_type := 'compile' | 'msg' 
     \endverbatim     
     */
    class IPragma : public IExpression {
	
	ParamList _params;
	
	static const std::string COMPILE;// = "compile";
	static const std::string MSG;// = "msg";

    public :

	/**
	 * \param token, the location and type of the pragma
	 * \params the parameters of the pragma 
	 */
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

	/**
	 * \brief execute a display at compile time
	 */
	void executeMsg ();

	/**
	 * \brief transform errors of compilation into boolean
	 * \return iif there is no errors, return a BoolValue (true), BoolValue (false) otherwise
	 */
	Expression executeCompile ();       
	
    };

    typedef IPragma* Pragma;
    
}
