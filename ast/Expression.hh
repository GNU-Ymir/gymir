#pragma once

#include "Instruction.hh"

#include "../errors/_.hh"
#include "../syntax/Word.hh"
#include <map>

namespace semantic {
    class ISymbol;
    typedef ISymbol* Symbol;

    class IInfoType;
    typedef IInfoType* InfoType;    
}

namespace syntax {

    class IType;

    /**
     * \struct IExpression
     * Parent class of all expression
     */
    class IExpression : public IInstruction {
    public:

	/** The type information of this expression */
	::semantic::Symbol info = NULL;

	/** The instruction where the expression is located, used the refer to different context  */
	Instruction inside;

	/**
	 * \param word the location of the expression
	 */
	IExpression (Word word) :
	    IInstruction (word)
	{}

	/**
	 * \brief cf. expression ();
	 */
	Instruction instruction () {
	    return this-> expression ();
	}

	/**
	 * \brief Perform the semantic analyses 
	 * \warning if this method is no overriden, an assert error will be thrown at execution time
	 */
	virtual IExpression* expression () {
	    this-> print (0);
	    Ymir::Error::assert ((std::string ("TODO expression () : ") + this-> getIds ().back ()).c_str ());
	    return NULL;
	}

	/**
	 * \brief Transform this expression to a type (if possible, throw language errors and return NULL otherwise)
	 */
	virtual IType* toType ();

	/**
	   \return could this expression be used as a type ?
	 */
	virtual bool isType ();

	/**
	 * \return could this expression be used as an expression (with a dynamic value ?)
	 */
	virtual bool isExpression ();

	/**
	   \brief cf. templateExpReplace (elems);
	 */
	IInstruction* templateReplace (const std::map <std::string, IExpression*>& elems) final {
	    return this-> templateExpReplace (elems);
	}
	
	/**
	 * \brief Replace the different occurence of the variable by template expression
	 * \brief Generally called after, template resolution
	 * \param values an associative array, string => Expression representing the template association
	 * \return an untyped expression where all occurence found in values are replaced
	 */
	virtual IExpression* templateExpReplace (const std::map <std::string, IExpression*>&);

	/**
	 * \return all the symbol of declarated variables 
	 */
	virtual std::vector <semantic::Symbol> allInnerDecls () {
	    return {};
	}

	/**
	 * \return a new allocated expression clone of this one
	 */
	IExpression* clone ();

	/**
	 * \brief Called by clone () function
	 * \warning if this method is no overriden, an assert error will be thrown at execution time
	 */
	virtual IExpression* onClone () {
	    this-> print (0);
	    Ymir::Error::assert ((std::string ("TODO") + this-> getIds ().back ()).c_str ());
	    return NULL;	    
	};
	
	static const char * id () {
	    return TYPEID (IExpression);
	}
	
	virtual std::vector <std::string> getIds () override {
	    auto ids = IInstruction::getIds ();
	    ids.push_back (TYPEID (IExpression));
	    return ids;
	}
	
	/**
	 * \brief Can this expression be used as an lvalue ?
	 * \return false, by default
	 */
	virtual bool isLvalue ();

	virtual void print (int) override;

	/**
	 * \return a string encoding the expression using the ymir language syntax
	 */
	virtual std::string prettyPrint () override;

	virtual ~IExpression ();
	
    };

    typedef IExpression* Expression;
    
}
