#pragma once
#include <ymir/utils/Ref.hh>
#include <ymir/errors/Error.hh>

namespace documentation {

    namespace json {

	class JsonValue;
	
	/**
	 * Store a json dumpable value
	 */
	class IJsonValue {
	private :

	    friend JsonValue;
	    
	public :

	    virtual JsonValue clone () const = 0;

	    	    
	    virtual std::string toString () const = 0;
	    
	};

	class JsonValue : public RefProxy<IJsonValue, JsonValue> {
	public : 

	    static JsonValue __empty__;

	public :

	    /**
	     * Same as empty
	     */
	    JsonValue ();
	    
	    JsonValue (IJsonValue * val);

	    /**
	     * Create an empty value
	     */
	    static JsonValue empty ();

	    /**
	     * \return true if this json value is empty
	     */
	    bool isEmpty () const;

	    /**
	     * Dump the json value to a string
	     */
	    std::string toString () const;

	    
	    /**
	     * \brief Cast the content pointer into the type (if possible)
	     * \brief Raise an internal error if that is not possible
	     */
	    template <typename T>
	    T& to ()  {
#ifdef DEBUG
		if (dynamic_cast<T*> (this-> _value.get ()) == nullptr)
		    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
#endif
		return *((T*) this-> _value.get ());	    
	    }

	    /**
	     * \brief Cast the content pointer into the type (if possible)
	     * \brief Raise an internal error if that is not possible
	     */
	    template <typename T>
	    const T& to () const  {
#ifdef DEBUG
		if (dynamic_cast<T*> (this-> _value.get ()) == nullptr)
		    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
#endif
		return *((const T*) this-> _value.get ());	    
	    }

	    /**
	     * \brief Tell if the inner type inside the proxy is of type T
	     */
	    template <typename T>
	    bool is () const  {	    
		return dynamic_cast<T*> (this-> _value.get ()) != nullptr;
	    }
	    
	};
	
    }    
}
