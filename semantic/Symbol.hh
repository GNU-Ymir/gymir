#pragma once

#include <ymir/utils/Ref.hh>
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
	/** This information is set at getting time */
	ISymbol* _referent;
       	
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
	 * \brief Change the name of the symbol
	 */
	void setName (const std::string & name);
	
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
	 * \param sym the symbol to insert
	 */
	virtual void insert (const Symbol & sym);

	/**
	 * \brief Insert a new symbol in the current one
	 * \brief It will work like insert, but ensure that there is only one occurence of the symbol
	 * \brief sym the symbol to insert 
	 */
	virtual void replace (const Symbol & sym);
	
	/**
	 * \brief Find a symbol named name, in the scope hierarchy
	 * \brief It will ask it's referent recursively
	 * \return a list of symbol, or an empty list if it does not exists
	 */
	virtual std::vector <Symbol> get (const std::string & name) const ;

	/**
	 * \brief Find a symbol named name in the table of this symbol
	 * \brief Does not call it's referent
	 * \return a symbol, may be empty
	 */
	virtual std::vector <Symbol> getLocal (const std::string & name) const;
	
	/**
	 * \brief In the symbol hierarchy, each symbol is attached to a referent
	 * It can occur that a symbol has no referent, it can mean two things : 
	 * - first, the symbol is not inserted anywhere (malformed)
	 * - second, the symbol is global and is a main referent 
	 * Only modules can be in second case
	 * \return the symbol in which the symbol can be retreived	 
	 * \return Can return an empty symbol, if this symbol is a global one (modules only)
	 */
	Symbol getReferent () const;

	/**
	 * \brief change the referent of the symbol
	 */
	void setReferent (const Symbol &sym);
	
	/**
	 * \return the space name of the symbol
	 */
	virtual std::string getRealName () const;
	
	/**
	 * \return is this symbol the same as other (no only address, or type)
	 */
	virtual bool equals (const Symbol & other) const = 0;
	
	/**
	 * \return a formated string of the hierarchy of the modules and all declared symbols
	 */
	virtual std::string formatTree (int padd) const = 0;
	
	/** Virtual but does not do anything */
	virtual ~ISymbol ();
    };
       

    /**
     * \struct Symbol
     * A symbol is anything that we can refer to at the semantic time
     * Or everything that can store informations about sub symbols (blocks for example) 
     */
    class Symbol : public RefProxy <ISymbol, Symbol> {
    public: 
	
	/** For convinience an empty symbol is declared to avoid segmentation fault on unreferenced symbol */
	static Symbol __empty__;

	/** This list containes the imported modules */
	static std::map <std::string, Symbol> __imported__;
	
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
	 * \brief Change the name of the symbol
	 */
	void setName (const std::string & name);
	
	/**
	 * Proxy function for symbol
	 */
	void insert (const Symbol & sym);

	/**
	 * Proxy function for symbol
	 */
	void replace (const Symbol & sym);
	
	/**
	 * Proxy function for symbol
	 */
	std::vector <Symbol> get (const std::string & name) const;

	/**
	 * Proxy function for symbol
	 */
	std::vector <Symbol> getLocal (const std::string & name) const;
	
	/**
	 * Proxy function for symbol
	 */
	Symbol getReferent () const;

	/**
	 * Proxy Function for symbol
	 */
	void setReferent (const Symbol & ref);

	/**
	 * \return the space name of the symbol
	 */
	std::string getRealName () const;
	
	/**
	 * Proxy function for symbol
	 */
	bool equals (const Symbol & other) const ;

	/**
	 * \return a formated string of the hierarchy of the modules and all declared symbols
	 */
	std::string formatTree (int padd = 0) const;
	
	/**
	 * \return The module named name, empty symbol if the module was not imported
	 */
	static const Symbol & getModule (const std::string & name);

	/**
	 * \brief Add a new known module (if the module already exists, it will erase it)
	 * \param name the name of the module
	 * \param sym the symbol containing the module
	 */
	static void registerModule (const std::string & name, const Symbol & sym);

	/**
	 * \brief Get a imported sub module, by its path name
	 * \param path the path to the module
	 * \return a module (or empty)
	 */
	static Symbol getModuleByPath (const std::string & path);

	/**Temporary function */
	static void clearModule ();
	
	/**
	 * \return The list of all declared modules
	 */
	static const std::map <std::string, Symbol> & getAllModules ();

	
	/**
	 * \brief Cast the content pointer into the type (if possible)
	 * \brief Raise an internal error if that is not possible
	 */
	template <typename T>
	T& to ()  {	    
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
	const T& to () const  {	    
	    if (this-> _value == NULL)
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "nullptr");	    

	    T t;
	    if (!this-> _value-> isOf (&t))
		Ymir::Error::halt (Ymir::ExternalError::get (Ymir::DYNAMIC_CAST_FAILED), "type differ");
	    return *((T*) this-> _value);	    
	}

	/**
	 * \brief Tell if the inner type inside the proxy is of type T
	 */
	template <typename T>
	bool is () const  {	    
	    if (this-> _value == NULL)
		return false;

	    T t;
	    return this-> _value-> isOf (&t); 			    
	}
	
    };
    
}
