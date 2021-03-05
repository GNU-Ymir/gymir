#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    class Mixin : public IDeclaration {
    	
	/** the name of the vars */
	Expression _mixin;

	std::vector <Declaration> _declarations;
	
    private :

	friend Declaration;

	Mixin ();

	Mixin (const lexing::Word & loc, const std::string & comment, const Expression & mixin, const std::vector <Declaration> & decls);

    public :

	/**
	 * \brief Create a new trait
	 * \param location the location of the use
	 * \param mixin the mixin to use
	 */
	static Declaration init (const lexing::Word & location, const std::string & comment, const Expression & mixin, const std::vector <Declaration> & declarations);

	const Expression & getMixin () const;

	const std::vector <Declaration> & getDeclarations () const;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };
    


}
