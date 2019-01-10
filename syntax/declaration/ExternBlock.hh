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

	bool isOf (const IDeclaration * type) const override;
	
    };

}
