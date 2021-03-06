#pragma once

#include <ymir/semantic/generator/Value.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Call
	 * Access a specific index inside an array
	 */
	class Call : public Value {

	    Generator _frame;

	    std::vector <Generator> _types;
	    
	    std::vector <Generator> _params;

	    std::vector <Generator> _addParams;
	    
	private :

	    friend Generator;
	    
	    Call ();

	    Call (const lexing::Word & loc, const Generator & type, const Generator & frame, const std::vector<Generator> & types, const std::vector <Generator> & params, const std::vector <Generator> & addParams = {});

	public :

	    /**
	     * \brief Generate a new affectation
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & frame, const std::vector<Generator> & types, const std::vector <Generator> & params, const std::vector <Generator> & addParams = {});
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /**
	     * \return the generator of the frame that will be called
	     */
	    const Generator & getFrame () const;

	    /**
	     * \return the type casters
	     */
	    const std::vector <Generator> & getTypes () const;
	    
	    /**
	     * \return the parameters
	     */
	    const std::vector <Generator> & getParameters () const;
	    
	    /**
	     * \return the list of additional params (only for C variadic call)
	     */
	    const std::vector <Generator> & getAddParameters () const;
	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
