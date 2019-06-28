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
    class DeclBlock : public IDeclaration {

	lexing::Word _token;

	std::vector <Declaration> _inner;

	bool _isPrivate;
	
    private :

	friend Declaration;

	DeclBlock ();

    public :

	static Declaration init (const DeclBlock & decl);
	
	static Declaration init (const lexing::Word & token, const std::vector <Declaration> & content, bool isPrivate);

	Declaration clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;	

	bool isOf (const IDeclaration * type) const override;

	const lexing::Word & getLocation () const;

	bool isPrivate () const;
	
	const std::vector <Declaration> & getDeclarations () const;
	
    };

}
