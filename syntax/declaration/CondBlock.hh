#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>


namespace syntax {

    /**
     * \struct CondBlock
     */
    class CondBlock : public IDeclaration {

	std::vector <Declaration> _inner;

	Declaration _else;

	syntax::Expression _test;
	
    private :

	friend Declaration;

	CondBlock ();

	CondBlock (const lexing::Word & token, const std::string & comment, const Expression & test, const std::vector <Declaration> & content, const Declaration & else_);

    public :

	static Declaration init (const CondBlock & decl);
	
	static Declaration init (const lexing::Word & token, const std::string & comment, const Expression & test, const std::vector <Declaration> & content, const Declaration & else_);

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;	

	bool isOf (const IDeclaration * type) const override;
		
	const std::vector <Declaration> & getDeclarations () const;

	const Expression & getTest () const;

	const Declaration & getElse () const;
	
    };

}
