#pragma once

#include <ymir/semantic/generator/Type.hh>
#include <ymir/errors/_.hh>

namespace semantic {
    namespace generator {

	class ErrorType : public Type {

	    std::string _name;

	    std::list <Ymir::Error::ErrorMsg> _errors;
	    
	public :

	    static std::string NAME;
	    
	private :

	    friend Generator;

	    ErrorType ();	    

	    ErrorType (const lexing::Word & loc, const std::string & name, const std::list <Ymir::Error::ErrorMsg> & errors);
	    
	public :

	    static Generator init (const lexing::Word & loc, const std::string & name = "none", const std::list <Ymir::Error::ErrorMsg> & errors = {});

	    Generator clone () const override;

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;

	    /**
	     * \return the name of the type
	     */
	    std::string typeName () const override;

	    const std::list <Ymir::Error::ErrorMsg> & getErrors () const;
	    
	};
       
    }
}
