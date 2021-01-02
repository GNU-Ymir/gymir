#pragma once

#include <ymir/semantic/generator/Value.hh>
#include <ymir/semantic/generator/Frame.hh>
#include <ymir/syntax/Expression.hh>

namespace semantic {

    namespace generator {

	/**
	 * \brief The LambdaProto unlike FrameProto store a uncomplete prototype to a lambda 
	 * \brief It stores a syntaxic element that will be validated only when it will be called
	 * \brief Its validation (done in the validator::Visitor will create a FrameProto and the frame that is associated
	 * \brief LambdaProto cannot have access to closure and are completely cte 
	 */
	class LambdaProto : public Value {

	    std::vector <Generator> _params;
	    
	    Generator _type;

	    std::string _name;

	    Frame::ManglingStyle _style = Frame::ManglingStyle::Y;

	    std::string _mangleName;

	    syntax::Expression _content;

	    bool _isRefClosure;

	    bool _isMoveClosure;

	    bool _index; // Cf closure
	    
	private :

	    friend Generator;
	    
	    LambdaProto ();

	    LambdaProto (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector <Generator> & params, const syntax::Expression & content, bool isRefClosure, bool isMoveClosure, uint index);

	public :

	    /**
	     * \brief Generate a new Binary on int
	     * \warning left and right operand must generate int typed values
	     */
	    static Generator init (const lexing::Word & loc, const std::string & name, const Generator & type, const std::vector <Generator> & params, const syntax::Expression & content, bool isRefClosure, bool isMoveClosure, uint index);

	    static Generator init (const LambdaProto & other, const std::string & mangleName, Frame::ManglingStyle style);
	    
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
	     * \return the name of the frame associated with the prototype
	     */
	    const std::string & getName () const;

	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    std::string prettyString () const override;

	    /**
	     * \return the mangling style of the frame
	     */
	    Frame::ManglingStyle getManglingStyle () const;



	    const std::string & getMangledName () const;

	    /**
	     * \return the content of the lambda proto
	     */
	    const syntax::Expression & getContent () const;

	    /**
	     * Is this lambda proto refer to a ref closure
	     */
	    bool isRefClosure () const;

	    /**
	     * Is this lambda refer to a lambda closure
	     */
	    bool isMoveClosure () const;

	    /**
	     * \return the index of the closure
	     * \Cf generator::Closure
	     */
	    uint getClosureIndex () const;
	    
	private :

	    static std::vector <Generator> getTypes (const std::vector <Generator> & gens);
	    
	};
	
    }

}
