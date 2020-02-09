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
	    
	private :

	    friend Generator;

	    MethodProto ();

	    MethodProto (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector <Generator> & params, bool isCVariadic, const Generator & classType, bool isMutable, bool isEmptyFrame, bool isFinal);

	public :
	    
	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector <Generator> & params, bool isCVariadic, const Generator & classType, bool isMutable, bool isEmptyFrame, bool isFinal);
	    
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
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    std::string prettyString () const override;

	    const Generator & getClassType () const;

	    bool isMutable () const;

	    bool isEmptyFrame () const;

	    bool isFinal () const;
	    
	};
    }

}
