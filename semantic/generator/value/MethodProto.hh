#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/value/FrameProto.hh>

namespace semantic {

    namespace generator {
	class MethodProto : public FrameProto {

	    bool _isMut;

	    Generator _classType;

	    /**
	     * True if the method prototype refer to a method that has no body
	     */
	    bool _isEmptyFrame; 

	    /**
	     * True if the method is final 
	     */
	    bool _isFinal;

	    /** This method proto is written inside a trait, and is here because we implement it whithout over */
	    bool _isFromImpl;
	    
	    std::string _comments;
	    
	private :

	    friend Generator;

	    MethodProto ();

	    MethodProto (const lexing::Word & loc, const std::string & comments, const std::string & name, const Generator & type, const std::vector <Generator> & params, bool isCVariadic, const Generator & classType, bool isMutable, bool isEmptyFrame, bool isFinal, bool isSafe, bool isFromImpl, const std::vector <Generator> & throwers);

	public :
	    
	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const std::string & comments, const std::string & name, const Generator & type, const std::vector <Generator> & params, bool isCVariadic, const Generator & classType, bool isMutable, bool isEmptyFrame, bool isFinal, bool isSafe, bool isFromImpl, const std::vector <Generator> & throwers);
	   	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    Generator clone () const  override;
	    
	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    bool equals (const Generator & other) const override;	    

	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    std::string prettyString () const override;

	    const Generator & getClassType () const;

	    bool isMutable () const;

	    bool isEmptyFrame () const;

	    bool isFinal () const;

	    bool isFromTrait () const;

	    const std::string & getComments () const;
	    
	};
    }

}
