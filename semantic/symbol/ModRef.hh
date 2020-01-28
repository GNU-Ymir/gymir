#pragma once

#include <ymir/semantic/Symbol.hh>
#include <ymir/semantic/Table.hh>
#include <vector>

namespace semantic {

    /**
     *  Define a reference to a module
     * This is generated by a import instruction
     */
    class ModRef : public ISymbol {

	std::string _name;

	Table _table;
	
    private :

	friend Symbol;

	ModRef ();

	ModRef (const lexing::Word & loc, const std::string & name);

	ModRef (const ModRef & mod);
	
    public :

	
	static Symbol init (const lexing::Word & loc, const std::string & name);
	
	static Symbol init (const lexing::Word & loc, const std::vector <std::string> & names);

	void insert (const Symbol & sym) override;

	void insertTemplate (const Symbol & sym) override;
	
	std::vector<Symbol> getTemplates () const override;
	
	std::vector <Symbol> getLocal (const std::string & name) const override;

	std::vector <Symbol> getLocalPublic (const std::string & name) const override;

	Symbol clone () const override;

	bool isOf (const ISymbol * type) const override;
	
	bool equals (const Symbol & other, bool parent = true) const override;

	/**
	 * \brief Merge two mod reference (assuming they have the same name)
	 * \brief Merge the branch having the same, name 
	 */
	Symbol merge (const ModRef & right) const;
	
	const std::string & getModName () const;

	/**
	 * \return the associated module if this is a leaf (ref to a real module), or this
	 */
	Symbol getModule () const;
	
	/**
	 * \return the space name of this symbol
	 */
	std::string getRealName () const override;
	
	std::string formatTree (int padd) const override;
	
    };

}
