#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>


namespace syntax {

    /**
     * \struct CondBlock
     * Just a convinient way to regroup a list of declaration
     */
    class CondBlock : public IDeclaration {

	lexing::Word _token;

	std::vector <Declaration> _inner;

	Declaration _else;

	syntax::Expression _test;
	
    private :

	friend Declaration;

	CondBlock ();

    public :

	static Declaration init (const CondBlock & decl);
	
	static Declaration init (const lexing::Word & token, const Expression & test, const std::vector <Declaration> & content, const Declaration & else_);

	Declaration clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;	

	bool isOf (const IDeclaration * type) const override;

	const lexing::Word & getLocation () const;
		
	const std::vector <Declaration> & getDeclarations () const;

	const Expression & getTest () const;

	const Declaration & getElse () const;
	
    };

}
