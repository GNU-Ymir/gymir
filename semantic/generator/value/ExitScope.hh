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
	    
	    std::vector<Generator> _success;

	    std::vector<Generator> _failure;

	    std::vector<Generator> _catchingVars;

	    std::vector<Generator> _catchingInfoType;

	    std::vector<Generator> _catchingActions;

	private :

	    friend Generator;
	    
	    ExitScope ();

	    ExitScope (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator & jmpBufType, const std::vector<Generator> & success, const std::vector<Generator> & failure, const std::vector<Generator> & catchingVar, const std::vector<Generator> & catchingInfos, const std::vector<Generator> & catchingActions);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator& jmpBufType, const std::vector <Generator> & values, const std::vector<Generator> & failure, const std::vector<Generator> & catchingVar, const std::vector<Generator> & catchingInfos, const std::vector<Generator> & catchingActions);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    bool isOf (const IGenerator * type) const override;	    
	    
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
	    const std::vector <Generator> & getSuccess () const;

	    /**
	     * \return the values to execute at the exit of the scope in case of failure
	     */
	    const std::vector <Generator> & getFailure () const;

	    /**
	     * \return the catching vars 
	     */
	    const std::vector <Generator> & getCatchingVars () const;

	    /**
	     * \return the catching info types
	     */
	    const std::vector <Generator> & getCatchingInfoTypes () const;
	    
	    /**
	     * \return the catching actions
	     */
	    const std::vector <Generator> & getCatchingActions () const;

	    /**
	     * \return the jump_buf type
	     */
	    const Generator & getJmpbufType () const;
	    
	    std::string prettyString () const override;
	    
	};
	
    }

}
