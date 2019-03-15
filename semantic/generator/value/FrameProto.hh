#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/Frame.hh>

namespace semantic {

    namespace generator {
	class FrameProto : public Value {

	    std::vector <Generator> _params;
	    
	    Generator _type;

	    std::string _name;

	    Frame::ManglingStyle _style = Frame::ManglingStyle::Y;
	    
	private :

	    friend Generator;
	    
	    FrameProto ();

	    FrameProto (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector <Generator> & params);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector <Generator> & params);
	    
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
	     * \return the return type of the frame prototype
	     */
	    const Generator & getReturnType () const;
		
	    /**
	     * \return the parameters of the frame
	     */
	    const std::vector<Generator> & getParameters () const;

	    /**
	     * \return the name of the frame associated with the prototype
	     */
	    const std::string & getName () const;

	    std::string prettyString () const override;


	    /**
	     * \return the mangling style of the frame
	     */
	    Frame::ManglingStyle getManglingStyle () const;

	    /**
	     * \brief Change the mangling style of the frame 
	     */
	    void setManglingStyle (Frame::ManglingStyle style);
	    
	    
	};
	
    }

}
