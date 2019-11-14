#pragma once

#include <ymir/semantic/Symbol.hh>
#include <map>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>

namespace semantic {


    class Table;
    
    /**
     * \brief The class Table is used to store the symbols declared somewhere
     * \brief It can be anything that derived from type Symbol
     */
    class Table {

	/** 
	 * All the symbol declared inside the current table 
	 * The order of insertion is respected
	 * When new insertion is made, it replace the ancient symbol 
	 * It means that if we have inserted symbol the following way : 
	 *  - [d : 1, b : 2, c : 8], 
	 * When inserting b : 9, the result will be  
	 *  - [d : 1, b : 9, c : 8]
	 */
	std::vector <Symbol> _syms;

	/**
	 * All the template symbol declared inside the current table
	 *
	 */
	std::vector <Symbol> _templates;

	/** 
	 * The symbol possessing this table 
	 * This symbol cannot be nullptr
	 */
	ISymbol* _attached; 
		
    public :

	/**
	 * Please use it only for construction of Table field (see example in semantic/symbol/Class)
	 * This class is a special one, that cannot be proxied, and cannot act as if it is
	 */
	Table (ISymbol * attached);
	
	/**
	 * \brief Create an empty table
	 * \warning if attached is empty, an internal excpetion is raised
	 */
	static Table init (ISymbol * attached);	

	/**
	 * \brief make a copy of the table, and all symbols
	 */
	Table clone (ISymbol * attached) const;

	/**
	 * \return the symbol in which the table is attached
	 */
	ISymbol* getAttach ();

	/**
	 * \brief Change the attach of the table
	 * \param attach the new attach
	 */
	void setAttach (ISymbol * attach);
	
	/**
	 * \brief Insert a new symbol in the table
	 * \param sym the symbol to insert
	 */
	void insert (const Symbol & sym);

	/**
	 * \brief Insert a new template symbol in the table
	 * \brief Unlike classic insert, this symbol cannot be retreived with get
	 * \param sym the symbol to insert
	 */
	void insertTemplate (const Symbol & sym);
	
	/**
	 * \brief Insert a new symbol in the table
	 * \brief if the symbol already exists, it will replace it
	 * \param sym the symbol to insert
	 */
	void replace (const Symbol & sym);
	
	/**
	 * \brief Get a symbol from this table
	 * \brief As you can see in the type returned, you only can get symbol by value
	 * \brief If you want to modify a symbol inside the table, you will need to reinsert it once modified
	 * \brief It can happens sometimes, that you don't know from which table the symbol comes from
	 * \brief This informations can be retreived inside the symbol
	 * \param name the name of the symbol
	 */
	std::vector <Symbol> get (const std::string & name) const;

	/**
	 * \brief Get a symbol from this table
	 * \brief As you can see in the type returned, you only can get symbol by value
	 * \brief If you want to modify a symbol inside the table, you will need to reinsert it once modified
	 * \brief It can happens sometimes, that you don't know from which table the symbol comes from
	 * \brief This informations can be retreived inside the symbol
	 * \brief Only return symbol that are marked public
	 * \param name the name of the symbol
	 */	
	std::vector <Symbol> getPublic (const std::string & name) const;
	
	/**
	 * \return the list of all declared symbol inside the table
	 */
	const std::vector <Symbol> & getAll () const;
	
    };    
    
}
