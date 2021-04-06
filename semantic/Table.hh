#pragma once

#include <ymir/semantic/Symbol.hh>
#include <map>
#include <ymir/errors/Error.hh>
#include <ymir/errors/ListError.hh>
#include <unordered_map>

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
	std::unordered_map <std::string, std::vector <Symbol> > _syms;
	

	std::unordered_map <std::string, std::vector <Symbol> > _public_syms;

	std::vector <Symbol> _all;
	
	/**
	 * All the template symbol declared inside the current table
	 *
	 */
	std::vector <Symbol> _templates;

	/** 
	 * The symbol possessing this table 
	 * This symbol cannot be nullptr
	 */
	std::weak_ptr <ISymbol> _attached;
		
    public :

	Table (const Symbol & attach); // I wish i could put that private but shared pointer ...

	static std::shared_ptr<Table> init (const Symbol & attach);
	
	// /**
	//  * \brief make a copy of the table, and all symbols
	//  */
	// std::shared_ptr<Table> clone (const Symbol & attached) const;

	/**
	 * \return the symbol in which the table is attached
	 */
	Symbol getAttach ();
	
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
	 * \return the list of inserted templates
	 */
	const std::vector<Symbol> & getTemplates () const;
	
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
	void get (const std::string & name, std::vector <Symbol> & ret) const;

	/**
	 * \brief Get a symbol from this table
	 * \brief As you can see in the type returned, you only can get symbol by value
	 * \brief If you want to modify a symbol inside the table, you will need to reinsert it once modified
	 * \brief It can happens sometimes, that you don't know from which table the symbol comes from
	 * \brief This informations can be retreived inside the symbol
	 * \brief Only return symbol that are marked public
	 * \param name the name of the symbol
	 */	
	void getPublic (const std::string & name, std::vector <Symbol>  &) const;
	
	/**
	 * \return the list of all declared symbol inside the table
	 */
	const std::vector <Symbol> & getAll () const;


    private :

	void recomputeAll ();
	
    };    
    
}
