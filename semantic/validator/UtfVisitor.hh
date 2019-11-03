#pragma once

#include <ymir/semantic/validator/Visitor.hh>

namespace semantic {

    namespace validator {

	class UtfVisitor {

	    Visitor & _context;

	private :

	    /**
	     * \brief As for any not proxied class, it does nothing special
	     * \brief We just wan't all class to be initialized the same way
	     */
	    UtfVisitor (Visitor & context);

	public :

	    /**
	     * \brief Create a new Visitor
	     * \param context the context of the bracket visitor (used to validate the escape chars)
	     */
	    static UtfVisitor init (Visitor & context);


	    /**
	     * \brief Convert a single char to its unicode
	     * \param loc the location of the char
	     * \param content the content of the char literal
	     * \param size the type of the char (8, 16, 32)
	     */
	    uint convertChar (const lexing::Word & loc, const lexing::Word & content, int size);


	    /**
	     * \brief Convert a string literal to an unicode string
	     * \param loc the location of the string
	     * \param content the content o the string literal
	     * \param size the type of the inner content (8, 16, 32)
	     */
	    std::vector<char> convertString (const lexing::Word& loc, const lexing::Word & content, int size, int & len);

	private :

	    size_t utf8_codepoint_size (uint8_t text);
	    
	    std::vector <uint> utf8_to_utf32 (const std::string& text);

	    void getUnicodeChar (int & nb, uint code, char chars[5]);

	    void escapeUnicode (const lexing::Word & loc, int & it, const std::string & content, Ymir::OutBuffer & buf, const std::string & size);

	    std::vector<char> toString (const std::vector <uint> & content);
	    
	    std::string escapeChar (const lexing::Word & loc, const std::string & content, const std::string & size);	    
	    
	};
	
    }

}
