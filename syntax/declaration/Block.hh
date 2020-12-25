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

	std::vector <Declaration> _inner;

	bool _isPrivate;

	bool _isProt;
	
    private :

	friend Declaration;

	DeclBlock ();

	DeclBlock (const lexing::Word & loc, const std::string & comment, const std::vector <Declaration> & content, bool isPrivate, bool isProt);

    public :

	static Declaration init (const DeclBlock & decl);
	
	static Declaration init (const lexing::Word & token, const std::string & comment, const std::vector <Declaration> & content, bool isPrivate, bool isProt);

	void treePrint (Ymir::OutBuffer & stream, int i = 0) const override;	

	bool isOf (const IDeclaration * type) const override;

	bool isPrivate () const;

	bool isProt () const;

	bool isPublic () const;
	
	const std::vector <Declaration> & getDeclarations () const;
	
    };

}
