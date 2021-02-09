#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	class Cast : public Value {

	    Generator _who;

	private :

	    friend Generator;
	    
	    Cast ();

	    Cast (const lexing::Word & loc, const Generator & type, const Generator & who);

	public :

	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & who);
	    
	    Generator clone () const  override;

	    bool equals (const Generator & other) const override;	    

	    const Generator & getWho () const;

	    std::string prettyString () const override;
	};
	
    }

}
