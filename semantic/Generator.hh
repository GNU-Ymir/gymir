#pragma once

#include <ymir/utils/Ref.hh>
#include <ymir/lexing/Word.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>


namespace semantic {

    /**
     * This modules defined the generator elements
     * A generator is a element that can produce code, whithout referering to external symbol
     * For example a frame is a generator that will produce intermediate language frame (gimple frame)
     * It does not need anything that comes from outside the frame
     */
    namespace generator {

	class Generator;

	/**
	 * \struct IGenerator
	 * Ancestor of all kind of generator
	 */
	class IGenerator {
	private :

	    /** 
	     * The location of the generator in the source file 
	     * At this level, this does not contain the name of the element anymore
	     */
	    lexing::Word _location;

	    /** The name of the generator */
	    std::string _name;

	    /**
	     * The location of the thrower if any
	     */
	    std::vector<Generator> _throwers;
	    
	    
	    /** The id of the generator */
	    uint _uniqId; 

	    static uint __lastId__;
	    
	private :

	    friend Generator;

	    /** only used for dynamic cast */
	    IGenerator (); 

	public :

	    /** 
	     * \brief Create a generator
	     */
	    IGenerator (const lexing::Word & location, const std::string & name);

	    /** Copy construction */
	    IGenerator (const IGenerator & other);
	    
	    /** 
	     * \brief Mandatory function used inside proxy design pattern
	     */
	    virtual Generator clone () const = 0;

	    /**
	     * \brief Mandatory function used inside proxy design pattern for dynamic casting
	     */
	    virtual bool isOf (const IGenerator * type) const = 0;	    

	    /**
	     * \return is this generator the same as other (no only address, or type)
	     */
	    virtual bool equals (const Generator & other) const = 0;

	    /**
	     * \return the location of the generator for debuging info generation
	     */
	    const lexing::Word & getLocation () const;
	    
	    /**
	     * \return the name of the generator
	     */
	    const std::string & getName () const;

	    /**
	     * \return the id of the generator
	     */
	    virtual uint getUniqId () const;

	    /**
	     * \brief Change the id of the generator
	     * \brief This in principal is only used when we need to refer to VarDecl for which the type is not totally knwon yet (\ie Lambda closure)
	     */
	    void setUniqId (uint id);
	    
	    /**
	     * \brief Reset the count of uniq ids
	     */
	    static void resetIdCount ();

	    /**
	     * \return a uniq id
	     */
	    static uint getLastId (); 
	    
	    /**
	     * \return a formatted string representing the generator in Ymir like syntax
	     */
	    virtual std::string prettyString () const;

	    /**
	     * \brief Change the list of location that are throwing something that is not catched
	     */
	    void setThrowers (const std::vector <Generator> & loc);
	    
	    /**
	     * \return the list of location that are throwing something that is not catched
	     */
	    const std::vector <Generator> & getThrowers () const;

	    
	    /** Virtual but does not do anything */
	    virtual ~IGenerator ();
	    
	protected :
	    
	    /**
	     * Change the location of the generator (useful for templates)
	     */
	    void changeLocation (const lexing::Word & loc);
	    
	};

	class Generator : public RefProxy <IGenerator, Generator> {
	public : 

	    /** For convinience an empty generator is declared to avoid segmentation fault on unreferenced generator */
	    static Generator __empty__;

	    static std::vector <Generator> __nothrowers__;
	    
	public :

	    Generator (IGenerator * gen);

	    static Generator init (const lexing::Word & loc, const Generator & other);
	    
	    /**
	     * \brief Create en empty generator
	     */
	    static Generator empty ();

	    /**
	     * \return is this generator empty
	     */
	    bool isEmpty () const;

	    /**
	     * Proxy function for generator
	     */
	    bool equals (const Generator & other) const ;

	    /**
	     * Proxy function for generator
	     */
	    const lexing::Word & getLocation () const;
	    
	    /**
	     * Proxy function for generator
	     */
	    const std::string & getName () const;

	    /**
	     * Proxy function for generator
	     */
	    uint getUniqId () const;

	    /**
	     * Proxy function for generator
	     */
	    void setUniqId (uint id);
	    
	    /**
	     * Proxy function for generator
	     */
	    static void resetIdCount ();

	    
	    /**
	     * Proxy function for generator
	     */
	    static uint getLastId ();
	    
	    /**
	     * Proxy function for generator
	     */
	    std::string prettyString () const;

	    /**
	     * Proxy function for generator
	     */
	    void setThrowers (const std::vector <Generator> & loc);
	    
	    /**
	     * Proxy function for generator
	     */
	    const std::vector <Generator> & getThrowers () const;
	    
	    /**
	     * \brief Cast the content pointer into the type (if possible)
	     * \brief Raise an internal error if that is not possible
	     */
	    template <typename T>
	    T& to ()  {	    
		if (this-> _value == nullptr)
		    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");	    

		T t;
		if (!this-> _value.get ()-> isOf (&t))
		    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
		return *((T*) this-> _value.get ());	    
	    }

	    /**
	     * \brief Cast the content pointer into the type (if possible)
	     * \brief Raise an internal error if that is not possible
	     */
	    template <typename T>
	    const T& to () const  {	    
		if (this-> _value == nullptr)
		    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");	    

		T t;
		if (!this-> _value.get ()-> isOf (&t))
		    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
		return *((const T*) this-> _value.get ());	    
	    }

	    /**
	     * \brief Tell if the inner type inside the proxy is of type T
	     */
	    template <typename T>
	    bool is () const  {	    
		if (this-> _value == nullptr)
		    return false;

		T t;
		return this-> _value.get ()-> isOf (&t); 			    
	    }
	    
	};
	
    }    

}
