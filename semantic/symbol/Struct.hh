#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Struct.hh>
#include <ymir/semantic/Generator.hh>
#include <vector>

namespace semantic {

    /**
     * This symbol is a link between the syntaxic declaration of a structure 
     * And its symbolic representation
     * It is used (as a semantic::Function) to create a prototype of the structure
     * And use this prototype to construct and use structure
     */
    class Struct : public ISymbol {
	
	/** Is this structure data packed (<=> _isUnion == false) */
	bool _isPacked;

	/** Is this structure an union */
	bool _isUnion;

	const std::vector <syntax::Expression> _fields;

	/** Set at validation time, to prevent multiple time validation of the same symbol */
	generator::Generator _gen;
	
    private :

	friend Symbol;

	Struct ();

	Struct (const lexing::Word & name, const std::string & comments, const std::vector <syntax::Expression> & fields, bool isWeak);
	
    public :

	static Symbol init (const lexing::Word & name, const std::string & comments, const std::vector <syntax::Expression> & fields, bool isWeak);

	bool isOf (const ISymbol * type) const override;

	/**
	 * \brief The list of fields stores syntaxic nodes, because the types of fields cannot be known before validation
	 * \brief All the types must be declared before trying to type the variables 
	 * \verbatim
	 struct 
	 | a : A // A is declared after B, but we need it here
	 -> B;

	 struct 
	 | x : i32
	 -> A;
	 \endverbatim
	 * \return the list of fields of the structure	 
	 */
	const std::vector <syntax::Expression> & getFields () const;
	
	bool equals (const Symbol & other, bool parent) const override;

	/**
	 * \brief Set the pack information of the structure
	 * \brief A pack structure stores its information aligned by 1 byte
	 */
	void isPacked (bool isPacked);

	/**
	 * \return Does this structure reprensent a pack record type ?
	 */
	bool isPacked () const;

	/**
	 * \brief Set the union information of the structure
	 */
	void isUnion (bool isUnion);

	/**
	 * \return is this an union ?
	 */
	bool isUnion () const;
	
	std::string formatTree (int padd) const override;

	/**
	 * \brief This information is set at validation time
	 * \return the generator (structure prototype)
	 */
	const generator::Generator & getGenerator () const;

	/**
	 * \brief This information is set at validation time
	 * \brief set the generator (structure prototype)
	 */
	void setGenerator (const generator::Generator & gen);
	
    };

}


