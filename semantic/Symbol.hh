#pragma once

#include <ymir/utils/Proxy.hh>
#include <ymir/lexing/Word.hh>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>

namespace semantic {

    class Table;
    
    class Symbol;

    /**
     * \struct ISymbol
     * Ancestor of all kind of symbol
     */
    class ISymbol {
    private :
	
	lexing::Word _name;

	/** The symbol in which the symbol is declared */
	/** This information is set at insertion time */
	Symbol * _referent;
	
    private :
	
	/** Only used for dynamic cast */
	ISymbol ();

	friend Symbol;
	
    public :

	/**
	 * \brief A symbol is commonly attached to a real element 
	 * \brief For example a variable
	 * \param name is the name and location of the symbol
	 */
	ISymbol (const lexing::Word & name);

	/**
	 * \return the name and location of the symbol
	 */
	const lexing::Word & getName () const;
	
	/** 
	 * \brief Mandatory function used inside proxy design pattern
	*/
	virtual Symbol clone () const = 0;

	/**
	 * \brief Mandatory function used inside proxy design pattern for dynamic casting
	 */
	virtual bool isOf (const ISymbol * type) const = 0;

	/** 
	 * \brief Insert a new symbol in the current one
	 * \brief This is working for some kind of symbol only
	 * \brief If this function is not overriden, it will result into an exception throw
	 * \param sym the symbol to launch
	 */
	virtual void insert (const Symbol & sym);

	/**
	 * \brief Find a symbol named name, in the scope hierarchy
	 * \brief If this symbol does not know any symbol named name, 
	 * \brief It will ask it's referent 
	 * \return the symbol or a empty one, if it does not exists
	 */
	virtual const Symbol & get (const std::string & name) const ;
	
	/**
	 * \brief In the symbol hierarchy, each symbol is attached to a referent
	 * It can occur that a symbol has no referent, it can mean two things : 
	 * - first, the symbol is not inserted anywhere (malformed)
	 * - second, the symbol is global and is a main referent 
	 * Only modules can be in second case
	 * \return the symbol in which the symbol can be retreived	 
	 * \return Can return an empty symbol, if this symbol is a global one (modules only)
	 */
	const Symbol & getReferent () const;

	/**
	 * \brief A mutable version 
	 * \deprecated 
	 */
	Symbol& getReferent ();
	
	/** Virtual but does not do anything */
	virtual ~ISymbol ();
    };


    /**
     * \struct Symbol
     * A symbol is anything that we can refer to at the semantic time
     * Or everything that can store informations about sub symbols (blocks for example) 
     */
    class Symbol : public Proxy <ISymbol, Symbol> {
    public: 
	
	/** For convinience an empty symbol is declared to avoid segmentation fault on unreferenced symbol */
	static Symbol __empty__;
	
    public:

	Symbol (ISymbol * sym);
	
	/**
	 * \brief Create an empty symbol
	 */
	static Symbol empty ();

	/**
	 * \return is this symbol empty ?
	 */
	bool isEmpty () const;
	
	/**
	 * Proxy function for symbol
	 */
	const lexing::Word & getName () const;

	/**
	 * Proxy function for symbol
	 */
	void insert (const Symbol & sym);

	/**
	 * Proxy function for symbol
	 */
	const Symbol & get (const std::string & name) const;
	
	/**
	 * \brief Cast the content pointer into the type (if possible)
	 * \brief Raise an internal error if that is not possible
	 */
	template <typename T>
	T& to () {	    
	    if (this-> _value == NULL)
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");	    

	    T t;
	    if (!this-> _value-> isOf (&t))
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
	    return *((T*) this-> _value);	    
	}

		/**
	 * \brief Cast the content pointer into the type (if possible)
	 * \brief Raise an internal error if that is not possible
	 */
	template <typename T>
	const T& to () const {	    
	    if (this-> _value == NULL)
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");	    

	    T t;
	    if (!this-> _value-> isOf (&t))
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
	    return *((T*) this-> _value);	    
	}

    };
    
}
