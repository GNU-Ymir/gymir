#pragma once

#include "Instruction.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IReturn 
     * The syntaxic node representation of a return statement
     * \verbatim
     return := 'return' (expression)? ';'
     \endverbatim
     */
    class IReturn : public IInstruction {

	/** The element to return, may be null */
	Expression _elem;

	/** the cast to apply to the element (if not null of course) */
	semantic::InfoType _caster;

	/** ?? */
	bool _isUseless = false;
	
    public:

	/** 
	 * \param ident the location of the statement
	 */
	IReturn (Word ident);

	/** 
	 * \param ident the location of the statement
	 * \param elem the element to return
	 */
	IReturn (Word ident, Expression elem);

	/**
	 * \return the element that will be returned
	 */
	Expression getExpr ();
	
	Instruction instruction () override;

	Instruction templateReplace (const std::map <std::string, Expression>&) override;

	std::vector <semantic::Symbol> allInnerDecls () override;

	/**
	 * \return get or set the caster that will be used to cast the element at lint time
	 */
	semantic::InfoType & caster ();
	
	/**
	 * \return the caster that will be used to cast the element at lint time
	 */
	semantic::InfoType getCaster ();
	
	Ymir::Tree toGeneric () override;

	/** ?? */
	bool& isUseless ();
	
	void print (int nb = 0) override;

	static std::string id () {
	    return TYPEID (IReturn);
	}
	
	virtual std::vector <std::string> getIds () {
	    auto ret = IInstruction::getIds ();
	    ret.push_back (TYPEID (IReturn));
	    return ret;
	}
	
	virtual ~IReturn ();

    private :

	/**
	 * \brief verify the return var is not local
	 * \bug does not really work
	 */
	void verifLocal ();
	
    };
    
}
