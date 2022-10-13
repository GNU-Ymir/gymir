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
	    uint32_t convertChar (const lexing::Word & loc, const lexing::Word & content, int32_t size);


	    /**
	     * \brief Convert a string literal to an unicode string
	     * \param loc the location of the string
	     * \param content the content of the string literal
	     * \param size the type of the inner content (8, 16, 32)
	     */
	    std::vector<char> convertString (const lexing::Word& loc, const lexing::Word & content, int32_t size, int32_t & len, bool error = true);

	    /**
	     * \brief Convert a string literal to an unicode string
	     * \param loc the location of the string
	     * \param content the content of the string literal
	     * \param size the type of the inner content (8, 16, 32)
	     */
	    std::vector<char> convertString (const lexing::Word& loc, const std::string & content, int32_t size, int32_t & len, bool error = true);

	    /**
	     * Transform a utf32 string into a utf8 string
	     */
	    static std::vector <char> utf32_to_utf8 (const std::vector<char> & utf32);

	    /**
	     * Transform a utf8 string into a utf32 string
	     */
	    std::vector <uint32_t> utf8_to_utf32 (const std::string& text);
	    
	private :

	    size_t utf8_codepoint_size (uint8_t text);
	    

	    static void getUnicodeChar (int32_t & nb, uint32_t code, char chars[5]);

	    void escapeUnicode (const lexing::Word & loc, int32_t & it, const std::string & content, Ymir::OutBuffer & buf, const std::string & size);

	    std::vector<char> toString (const std::vector <uint32_t> & content);
	    
	    std::string escapeChar (const lexing::Word & loc, const std::string & content, const std::string & size, bool error);	    
	    
	};
	
    }

}
