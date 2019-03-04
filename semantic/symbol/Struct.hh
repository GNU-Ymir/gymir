#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <ymir/syntax/declaration/Struct.hh>
#include <ymir/semantic/Generator.hh>
#include <vector>

namespace semantic {

    /**
     * This symbol is a bit dirty as it refers to both syntax and semantics
     * A enhancement can certainly be made here
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

	Struct (const lexing::Word & name, const std::vector <syntax::Expression> & fields);
	
    public :

	static Symbol init (const lexing::Word & name, const std::vector <syntax::Expression> & fields);

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;
	

	const std::vector <syntax::Expression> & getFields () const;
	
	bool equals (const Symbol & other) const override;

	void isPacked (bool isPacked);

	bool isPacked () const;

	void isUnion (bool isUnion);

	bool isUnion () const;
	
	std::string formatTree (int padd) const override;

	const generator::Generator & getGenerator () const;

	void setGenerator (const generator::Generator & gen);

	
    };

}


