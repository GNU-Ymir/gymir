#pragma once

#include <ymir/semantic/generator/Value.hh>


namespace semantic {

    namespace generator {
	/**
	 * \struct NamedGenerator
	 */
	class NamedGenerator : public Value {
	
	    Generator _content;
	
	private :

	    friend Generator;

	    NamedGenerator ();
	
	    NamedGenerator (const lexing::Word & loc, const Generator & content);

	public :

	    static Generator init (const lexing::Word & token, const Generator & content);

	    Generator clone () const override;

	    bool isOf (const IGenerator * type) const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    	    
	    /**
	     * \return the content of the expression
	     */
	    const Generator & getContent () const;

	};    
    }
}
