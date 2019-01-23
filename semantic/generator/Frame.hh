#pragma once

#include <ymir/semantic/Generator.hh>

namespace semantic {
    namespace generator {

	class Frame : public IGenerator {
	private :
	    
	    std::vector <Generator> _params;
	    
	    /** The type of the generator */
	    Generator _type;

	    Generator _content;
	    
	private :

	    friend Generator;
	    
	    Frame ();

	    Frame (const lexing::Word & location, const std::string & name, const std::vector <Generator> & params, const Generator & type, const Generator & content);

	public :

	    /**
	     * \brief Create a frame 
	     * \param location the location of the frame (for debug info)
	     * \param name the name of the frame
	     * \param params the parameters of the frame
	     * \param type the type of the frame
	     * \param content the content to execute when calling this frame
	     */
	    static Generator init (const lexing::Word & location, const std::string & name, const std::vector <Generator> & params, const Generator & type, const Generator & content);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const ;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    virtual bool isOf (const IGenerator * type) const ;	    

	    /**
	     * \return is this symbol the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const ;	    
	    
	    /** 
	     * \return the type of the var
	     */
	    const Generator & getType () const;

	    /**
	     * \return the parameters of the frame
	     */
	    const std::vector <Generator> & getParams () const;

	    /**
	     * \return the content of the frame
	     */
	    const Generator & getContent () const;
	    
	};
	
    }
}
