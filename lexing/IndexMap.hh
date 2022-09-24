#pragma once

#include <list>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <ymir/utils/types.hh>


namespace lexing {

    /**
     * A node contained in the index map
     */
    class Node {
    private :
	
	char _key;

	bool _isToken;
	
	std::unordered_map <char, std::shared_ptr<Node> > _heads;

    public : 

	Node (char key);

	/**
	 * Insert sub char for possible token
	 */
	void insert (ulong start, const std::string & tok);

	/**
	 * \return the length of the token present at the begining of the content
	 * \start the index of the cursor (assuming that content [start - 1] == this-> _key
	 */
	ulong getLength (ulong start, const std::string & content) const;

	/**
	 * Simply return the key of this node
	 */
	char getKey () const;

	/**
	 * Transform the index map into string for debugging purpose
	 */
	std::string prettyString (int padd) const;
	
    };    

    /**
     * An index map is a efficient way of searching token inside a string
     * It is able to perform a tokenizing of a string content into a list of string     
     */
    class IndexMap {
    private :
	
	std::unordered_map <char, std::shared_ptr<Node> > _heads;

    public :

	IndexMap (const std::vector <std::string> & tokens);

	/**
	 * Insert a new token in the map
	 * Complexity : 0 (n * log (m)) with n = |token| and m = |already inserted Tokens| 
	 */
	void insert (const std::string & token);

	/**
	 * Get the length of the next word in the string 
	 */
	ulong next (const std::string & str) const;

	/**
	 * Get the length of the next word in the string 
	 * \start the location of the cursor (reading str from there)
	 */
	ulong next (ulong start, const std::string & str) const;

	/**
	 * Tokenize a string with the tokens that are registered inside the map
	 * Complexity : O (n * log (m)) with n = |str| and m = |Tokens|
	 */
	std::list <std::string> tokenize (const std::string & str) const;

	/**
	 * Transform the index map into string for debugging purpose
	 */
	std::string prettyString () const;

    };
    

}
