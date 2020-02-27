#pragma once

#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/Expression.hh>
#include <ymir/lexing/Word.hh>
#include <vector>

namespace syntax {

    class Use : public IDeclaration {
    	
	/** the name of the vars */
	Expression _module;
	
    private :

	friend Declaration;

	Use ();

	Use (const lexing::Word & location, const Expression & module);
	
    public :
	
	/**
	 * \brief Create a new trait
	 * \param location the location of the use
	 * \param module the module to use
	 */
	static Declaration init (const lexing::Word & location, const Expression & module);

	/**
	 * Mandatory function for dynamic casting
	 */
	bool isOf (const IDeclaration * type) const override;
				
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getModule () const;
	
    };
    


}
