#pragma once

#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	class Option : public Type {
	public : 

	    const static std::string INIT_NAME;
	    const static std::string TYPE_FIELD;
	    const static std::string VALUE_FIELD;
	    const static std::string ERROR_FIELD;
	    
	private: 
	    
	    friend Generator;

	    Option ();

	    Option (const lexing::Word & loc, const Generator & innerType, const Generator & errorType);

	public : 
	    
	    static Generator init (const lexing::Word & loc, const Generator & innerType, const Generator & errorType);

	    Generator clone () const override;

	    bool directEquals (const Generator & other) const override;

	    bool isCompatible (const Generator & other) const override;

	    bool needExplicitAlias () const override;

	    bool containPointers () const override;

	    std::string typeName () const override;
	    
	};
	    
    }
    
}
