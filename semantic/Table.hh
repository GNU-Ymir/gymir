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
    class ITable {

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
	 * The symbol possessing this table 
	 * This symbol cannot be nullptr
	 */
	ISymbol* _attached; 

	/** 
	 * The proxy, this one will contain the attached but in referencing mode 
	 * meaning it does not have the right to free the attach
	 */
	Symbol _proxy;
	
    private : 

	friend Table;
	
	ITable (ISymbol * attached);

    public :
	
	/**
	 * \brief Create an empty table
	 * \warning if attached is empty, an internal excpetion is raised
	 */
	static Table init (ISymbol * attached);	

	/**
	 * \brief make a copy of the table, and all symbols
	 */
	Table clone ();

	/**
	 * \return the symbol in which the table is attached
	 */
	Symbol & getAttach ();

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
	 * \brief Get a symbol from this table
	 * \brief As you can see in the type returned, you only can get symbol by value
	 * \brief If you want to modify a symbol inside the table, you will need to reinsert it once modified
	 * \brief It can happens sometimes, that you don't know from which table the symbol comes from
	 * \brief This informations can be retreived inside the symbol
	 * \param name the name of the symbol
	 */
	std::vector <Symbol> get (const std::string & name) const;

	/**
	 * \return the list of all declared symbol inside the table
	 */
	const std::vector <Symbol> & getAll () const;
	
    };

    class Table : public Proxy <ITable, Table> {
    public  :

	/**
	 * \brief Create a proxy for a table value
	 * \param table the table 
	 */
	Table (ITable * table);

	/**
	 * \brief Proxy function for ITable
	 */
	static Table init (ISymbol * sym);
	
	/**
	 * \brief Create an empty table	 
	 */
	static Table empty ();

	/** 
	 * Proxy function for ITable
	 */
	void insert (const Symbol & sym);

	/** 
	 * Proxy function for ITable
	 */
	std::vector <Symbol>  get (const std::string & name) const;
	
	/**
	 * \return the list of all symbols declared inside the table
	 */
	const std::vector <Symbol> & getAll () const;
	
	/**
	 * Proxy function for ITable
	 */
	Symbol & getAttach ();
	
	/**
	 * Proxy function for ITable
	 */
	void setAttach (ISymbol * attach);
       

    };   
    
    
}
