#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>


namespace syntax {

    /**
     * \struct DeclBlock
     * Just a convinient way to regroup a list of declaration
     */
    class ExternBlock : public IDeclaration {

	lexing::Word _location;

	/** The language which has implemented the following declaration */
	lexing::Word _from;

	/** The namespace in the external language */
	lexing::Word _space;
	
	/** a decl block always public */
	Declaration _content;
       		
    private :

	friend Declaration;

	ExternBlock ();

    public :

	static Declaration init (const ExternBlock & decl);
	
	static Declaration init (const lexing::Word & location, const lexing::Word & from, const lexing::Word & space, const Declaration & content);

	Declaration clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;
	
	bool isOf (const IDeclaration * type) const override;

	const lexing::Word & getLocation () const;
	
	/**
	 * \return the declaration 
	 */
	const Declaration & getDeclaration () const;

	/**
	 * \return the space of the external item
	 */
	const lexing::Word & getSpace () const;

	/**
	 * \return the language of the external item
	 */
	const lexing::Word & getFrom () const;
    };

}
