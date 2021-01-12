#pragma once

#include <ymir/semantic/generator/value/Binary.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Binary 
	 * Transform a value to a reference to this value
	 * The value must be a lvalue
	 */
	class ExitScope : public Value {

	    Generator _who;

	    Generator _jmpBuf;

	    Generator _catchingVar;

	    Generator _catchingInfoType;
	    
	    Generator _catchingAction;

	    std::vector<Generator> _exit;

	    std::vector<Generator> _failure;
	    
	private :

	    friend Generator;
	    
	    ExitScope ();

	    ExitScope (const lexing::Word & loc, const Generator & type, const Generator & jmpBufType, const Generator & who, const std::vector<Generator> & exit, const std::vector<Generator> & failure, const Generator & catchingVar, const Generator & catchingInfo, const Generator & catchingAction);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator& jmpBufType, const std::vector <Generator> & exit, const std::vector<Generator> & failure, const Generator & catchingVar, const Generator & catchingInfo, const Generator & catchingAction);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /** 
	     * \return the left operand of the operation
	     */
	    const Generator & getWho () const;

	    /**
	     * \return the values to execute at the exit of the scope in case of success
	     */
	    const std::vector <Generator> & getExit () const;

	    /**
	     * \return the values to execute at the exit of the scope in case of failure
	     */
	    const std::vector <Generator> & getFailure () const;

	    /**
	     * \return the catching vars 
	     */
	    const Generator & getCatchingVar () const;

	    /**
	     * \return the catching info types
	     */
	    const Generator & getCatchingInfoType () const;
	    
	    /**
	     * \return the catching actions
	     */
	    const Generator & getCatchingAction () const;

	    /**
	     * \return the jump_buf type
	     */
	    const Generator & getJmpbufType () const;
	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
