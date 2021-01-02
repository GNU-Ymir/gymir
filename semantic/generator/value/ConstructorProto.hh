#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/Frame.hh>
#include <ymir/semantic/Symbol.hh>

namespace semantic {

    namespace generator {
	class ConstructorProto : public Value {

	    std::vector <Generator> _params;
	    
	    Generator _type;

	    std::string _name;
	    
	    std::string _mangleName;

	    std::weak_ptr <ISymbol> _ref;
	    
	private :

	    friend Generator;
	    
	    ConstructorProto ();

	    ConstructorProto (const lexing::Word & loc, const std::string & name, const Symbol & ref, const Generator & type, const std::vector <Generator> & params, const std::vector <Generator> &throwers);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const std::string & name, const Symbol & ref, const Generator & type, const std::vector <Generator> & params, const std::vector <Generator> &throwers);

	    static Generator init (const ConstructorProto & proto, const std::string & name);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the return type of the frame prototype
	     */
	    const Generator & getReturnType () const;
		
	    /**
	     * \return the parameters of the frame
	     */
	    const std::vector<Generator> & getParameters () const;

	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    std::string prettyString () const override;

	    /**
	     * \return the name of the prototype
	     */
	    const std::string & getName () const;
	    

	    const std::string & getMangledName () const;

	    
	    Symbol getRef () const;

	private :

	    static std::vector <Generator> getTypes (const std::vector <generator::Generator> & params);
	    
	};
	
    }

}
