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

	/** The comments of the symbol */
	std::string _comments;
	
	/** The symbol in which the symbol is declared */
	/** This information is set at getting time */
	std::weak_ptr <ISymbol> _referent;

	/** The list of usable symbols for symbol search */
	/** This list is intended to be used only in the proxy symbol */
	std::map <std::string, Symbol> _used;

	bool _isPublic = false;

	bool _isProtected = false;

	bool _isWeak = false;
	
    private :
		
	friend Symbol;
	
    public :

	/**
	 * \brief A symbol is commonly attached to a real element 
	 * \brief For example a variable
	 * \param name is the name and location of the symbol
	 */
	ISymbol (const lexing::Word & name, const std::string & comments, bool isWeak);

	/**
	 * \return the name and location of the symbol
	 */
	const lexing::Word & getName () const;

	/**
	 * The comments on the symbol (discovered at syntax time, and pass through by declaration_Visitor)
	 */
	const std::string & getComments () const;
		
	/** 
	 * \brief Insert a new symbol in the current one
	 * \brief This is working for some kind of symbol only
	 * \brief If this function is not overriden, it will result into an exception throw
	 * \param sym the symbol to insert
	 */
	virtual void insert (const Symbol & sym);

	/**
	 * \brief Insert a template symbol 
	 * \brief This is working for some kind of symbol only
	 * \brief Unlike classic insert, a template symbol cannot be retreived
	 * \para sym the symbol to insert
	 */
	virtual void insertTemplate (const Symbol & sym);

	
	/**
	 * \return all the inserted template solutions
	 */
	virtual void getTemplates (std::vector <Symbol> & ret) const;
	
	/**
	 * \brief Insert a new symbol in the current one
	 * \brief It will work like insert, but ensure that there is only one occurence of the symbol
	 * \brief sym the symbol to insert 
	 */
	virtual void replace (const Symbol & sym);
	

	/**
	 * \brief Allows the use of an externally declared symbol for searching (in get (string) function)
	 * \brief Basically this method is called when importing a module, or with use_stmt
	 * \param name the name of the symbol (its path)
	 * \param use this symbol possess a table of symbols, that we have the right to access (publically)		
	 */
	virtual void use (const std::string & name, const Symbol & use);
	
	/**
	 * \brief Does the opposite of use (name, _);
	 */
	virtual void unuse (const std::string & name);	

	/**
	 * \brief Set the symbol to public
	 */
	void setPublic ();

	/**
	 * \brief Is this symbol declared public ?
	 */
	bool isPublic () const;

	/**
	 * \brief Set the symbol to protected (applicable for inner class definition only)
	 */
	void setProtected ();

	/**
	 * \brief Is this symbol declared protected
	 */
	bool isProtected () const;

	/**
	 * \brief Does this sym has been validated by template resolution?
	 */
	void setWeak ();

	/**
	 * \return Does this sym has been validated by template resolution
	 */
	bool isWeak () const;
	
	/**
	 * \brief Find a symbol named name, in the scope hierarchy
	 * \brief It will ask it's referent recursively
	 * \return a list of symbol, or an empty list if it does not exists
	 */
	virtual void get (const std::string & name, std::vector <Symbol> & ret) const ;

	/**
	 * \brief Find a symbol named name, in the scope hierarchy
	 * \brief It will ask it's referent recursively
	 * \return a list of symbol, or an empty list if it does not exists
	 */
	virtual void getPrivate (const std::string & name, std::vector <Symbol> & ret) const final;

	/**
	 * \brief Find a symbol named name, in the scope hierarchy
	 * \brief It will ask to it's referent recursively
	 * \brief It can only return symbols that are public
	 * \return a list of symbols (all publics), or an empty list it it does not exists
	 */
	virtual void getPublic (const std::string & name, std::vector <Symbol> & ret) const;	
	
	/**
	 * \brief Find a symbol named name in the table of this symbol
	 * \brief Does not call it's referent
	 * \return a symbol, may be empty
	 */
	virtual void getLocal (const std::string & name, std::vector <Symbol> & ret) const;

	/**
	 * \brief Find a symbol named name in the table of this symbol if it is public
	 * \brief Does not call it's referent
	 * \return a symbol, may be empty
	 */
	virtual void getLocalPublic (const std::string & name, std::vector <Symbol> & ret) const;	
	
	/**
	 * \return the list of used symbols
	 */
	const std::map <std::string, Symbol> & getUsedSymbols () const;
	
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
	 * \brief Used for inner mangling only, for the moment, the only application is for template solution 
	 */
	virtual std::string getMangledName () const;
	
	/**
	 * \return is this symbol the same as other (no only address, or type)
	 */
	virtual bool equals (const Symbol & other, bool parent = true) const = 0;
	
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
	
	/** For convinience an empty symbol is declared to avoid segmentation fault on unreferenced symbol, and still be able to return ref */
	static Symbol __empty__;

	/** This list containes the imported modules */
	static std::map <std::string, Symbol> __imported__;	

	/** This list contains the modules indexed by path, it is filled by getModuleByPath */
	static std::map <std::string, Symbol> __fast_mod_access__;
	
    public:

	Symbol (ISymbol * sym);

	Symbol (const std::shared_ptr <ISymbol> & sym);

	Symbol (const std::weak_ptr <ISymbol> & sym);

	~Symbol ();
	
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
	const std::string & getComments () const;
	
	/**
	 * Proxy function for symbol
	 */
	void insert (const Symbol & sym);

	/**
	 * Proxy function for symbol
	 */
	void insertTemplate (const Symbol & sym);

	/**
	 * Proxy function for symbol
	 */
	void getTemplates (std::vector <Symbol> & ret) const;

	/**
	 * Proxy function for symbol
	 */
	std::vector <Symbol> getTemplates () const;
	
	/**
	 * Proxy function for symbol
	 */
	void replace (const Symbol & sym);

	/**
	 * Proxy function for symbol
	 */
	void use (const std::string & name, const Symbol & sym);
	
	/**
	 * Proxy function for symbol
	 */
	void unuse (const std::string & name);

	/**
	 * Proxy function for symbol
	 */
	void setPublic ();

	/**
	 * Proxy function for symbol
	 */
	bool isPublic () const;
	
	/**
	 * Proxy function for symbol
	 */
	void setProtected ();

	/**
	 * Proxy function for symbol
	 */
	bool isProtected () const;
	
	/**
	 * Proxy function for symbol
	 */
	void setWeak ();

	/**
	 * Proxy function for symbol
	 */
	bool isWeak () const;
	
	/**
	 * Proxy function for symbol
	 */
	void get (const std::string & name, std::vector <Symbol>  & ret) const;

	/**
	 * Proxy function for symbol
	 */
	std::vector <Symbol> get (const std::string & name) const;
	
	/**
	 * Proxy function for symbol
	 */
	void getPrivate (const std::string & name, std::vector <Symbol> & ret) const;

	/**
	 * Proxy function for symbol
	 */
	std::vector<Symbol> getPrivate (const std::string & name) const;

	/**
	 * Proxy function for symbol
	 */
	void getPublic (const std::string & name, std::vector <Symbol> & ret) const;

	/**
	 * Proxy function for symbol
	 */
	std::vector<Symbol> getPublic (const std::string & name) const;
	
	/**
	 * \warning not a proxy function
	 * \brief search a symbol from a used symbol (*this* is currently in a list used in another symbol)
	 */
	void getUsed (const std::string & name, std::vector <Symbol> & ret) const;

	/**
	 * \warning not a proxy function
	 * \brief search a symbol from a used symbol (*this* is currently in a list used in another symbol)
	 */
	std::vector <Symbol> getUsed (const std::string & name) const;

	/**
	 * Proxy function for symbol
	 */
	const std::map <std::string, Symbol> & getUsedSymbols () const;
	
	/**
	 * Proxy function for symbol
	 */
	void getLocal (const std::string & name, std::vector <Symbol> & ret) const;

	/**
	 * Proxy function for symbol
	 */
	std::vector <Symbol> getLocal (const std::string & name) const;
	
	/**
	 * Proxy function for symbol
	 */
	void getLocalPublic (const std::string & name, std::vector <Symbol> & ret) const;

	/**
	 * Proxy function for symbol
	 */	
	std::vector <Symbol> getLocalPublic (const std::string & name) const;
	
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
	 * \brief Used for inner mangling only, for the moment, the only application is for template solution 
	 */
	std::string getMangledName () const;
	
	/**
	 * Proxy function for symbol
	 */
	bool equals (const Symbol & other, bool parent = true) const ;

	/**
	 * \brief This function will check if the symbols are the same (in ref terms)
	 * \param other the symbol on which to test the equivalence
	 */
	bool isSameRef (const Symbol & other) const ;
	
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

	
	/**
	 * \return The list of all declared modules
	 */
	static const std::map <std::string, Symbol> & getAllModules ();

	/**
	 * Remove all the imported symbols
	 */
	static void purge ();	
	
	/**
	 * \brief Cast the content pointer into the type (if possible)
	 * \brief Raise an internal error if that is not possible
	 */
	template <typename T>
	T& to ()  {
#ifdef DEBUG
	    if (dynamic_cast <T*> (this-> _value.get ()) == nullptr)
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
	    if (dynamic_cast <T*> (this-> _value.get ()) == nullptr)
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
	
	/**
	 * \brief It can happens that symbols can be accessed by multiple points
	 * \brief This will result of a multiple definition of the same symbol
	 * \brief This function will merge all equivalent symbols to prevent this to happen
	 * \param multSym a list of symbols (basically returned by get or getLocal function)
	 * \return a list of uniq symbols
	 */
	static void mergeEqSymbols (std::vector <Symbol> & ret);	
    };
    
}
