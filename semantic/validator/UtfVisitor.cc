#include <ymir/semantic/validator/UtfVisitor.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/generator/value/_.hh>
#include <ymir/semantic/generator/type/_.hh>
#include <string>
#include <algorithm>

namespace semantic {

    namespace validator {

	using namespace Ymir;

	UtfVisitor::UtfVisitor (Visitor & context) : _context (context) {
	}

	UtfVisitor UtfVisitor::init (Visitor & context) {
	    return UtfVisitor (context);
	}
	
	
	size_t UtfVisitor::utf8_codepoint_size(uint8_t text) {
	    // According to utf-8 documentation, a continuous char begin with 10xxxxxx,
	    // Meaning 01101000 10111111 is 1 in length, and 0111111 11010100 is 2 in length
		    
	    if((text & 0b10000000) == 0) {
		return 1;
	    }

	    if((text & 0b11100000) == 0b11000000) {
		return 2;
	    }

	    if((text & 0b11110000) == 0b11100000) {
		return 3;
	    }
		    
	    return 4;
	}

	std::vector <uint> UtfVisitor::utf8_to_utf32(const std::string & text) {
	    std::vector <uint> res;
	    size_t i = 0;

	    for (size_t n = 0; i < text.length (); n++) {
		size_t byte_count = utf8_codepoint_size(text[i]);
			
		uint a = 0, b = 0, c = 0, d = 0;
		uint a_mask, b_mask, c_mask, d_mask;
		a_mask = b_mask = c_mask = d_mask = 0b00111111;
			
		switch(byte_count) {
		case 4 : {
		    a = text [i]; b = text [i + 1]; c = text [i + 2]; d = text [i + 3];
		    a_mask = 0b00000111;
		} break;
		case 3 : {
		    b = text [i]; c = text [i + 1]; d = text [i + 2];
		    b_mask = 0b00001111;
		} break;
		case 2 : {
		    c = text [i]; d = text [i + 1];
		    c_mask = 0b00011111;
		} break;
			
		case 1 : {
		    d = text [i];
		    d_mask = 0b01111111;
		} break;
		}
			
		uint b0 = a & a_mask;
		uint b1 = b & b_mask;
		uint b2 = c & c_mask;
		uint b3 = d & d_mask;
		res.push_back ((b0 << 18) | (b1 << 12) | (b2 << 6) | b3);

		i += byte_count;
	    }

	    return res;
	}

	void UtfVisitor::getUnicodeChar(int & nb, unsigned int code, char chars[5]) {
	    if (code <= 0x7F) {
		chars[0] = (code & 0x7F); chars[1] = '\0';
		nb = 1;
	    } else if (code <= 0x7FF) {
		// one continuation byte
		chars[1] = 0x80 | (code & 0x3F); code = (code >> 6);
		chars[0] = 0xC0 | (code & 0x1F);
		chars[2] = '\0';
		nb = 2;
	    } else if (code <= 0xFFFF) {
		// two continuation bytes
		chars[2] = 0x80 | (code & 0x3F); code = (code >> 6);
		chars[1] = 0x80 | (code & 0x3F); code = (code >> 6);
		chars[0] = 0xE0 | (code & 0xF); chars[3] = '\0';
		nb = 3;
	    } else if (code <= 0x10FFFF) {
		// three continuation bytes
		chars[3] = 0x80 | (code & 0x3F); code = (code >> 6);
		chars[2] = 0x80 | (code & 0x3F); code = (code >> 6);
		chars[1] = 0x80 | (code & 0x3F); code = (code >> 6);
		chars[0] = 0xF0 | (code & 0x7); chars[4] = '\0';
		nb = 4;
	    } else {
		// unicode replacement character
		chars[2] = 0xEF; chars[1] = 0xBF; chars[0] = 0xBD;
		chars[3] = '\0';
		nb = 3;
	    }
	}
		
	void UtfVisitor::escapeUnicode (const lexing::Word & loc, int & it, const std::string & content, OutBuffer & buf, const std::string & size) {
	    auto fst = content.find_first_of ('{');
	    auto scd = content.find_first_of ('}');
	    if (fst == std::string::npos || scd == std::string::npos)  {
		auto real_loc = loc;
		real_loc.column += it;
		Error::occur (real_loc, ExternalError::get (UNTERMINATED_SEQUENCE));
	    }

	    auto inner = content.substr (fst + 1, scd - fst - 1);
		    
	    auto fixed = syntax::Fixed::init (lexing::Word {loc, inner}, lexing::Word {loc, size});
	    auto gen = this-> _context.validateFixed (fixed.to<syntax::Fixed> (), 16);
	    auto ui = (uint) gen.to <semantic::generator::Fixed> ().getUI ().u;

	    char chars[5];
	    int nb = 0;
	    getUnicodeChar (nb, ui, chars);
	    for (int i = 0 ; i < nb; i++)			
		buf.write (chars[i]);
		    
	    it = scd + 1;

	}
	       		
	std::string UtfVisitor::escapeChar (const lexing::Word & loc, const std::string & content, const std::string & size) {
	    OutBuffer buf;
	    int it = 0;
	    static std::vector <char> escape = {'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\'', '\"', '"', '?'};
	    static std::vector <uint> values = {7, 8, 12, 10, 13, 9, 11, 92, 39, 34, 63};
		    
	    while (it < (int) content.size ()) {
		if (content [it] == '\\') {
		    if (it + 1 < (int) content.size ()) {
			it += 1;
			if (content [it] == 'u') escapeUnicode (loc, it, content, buf, size);
			else {
			    auto pos = std::find (escape.begin (), escape.end (), content [it]) - escape.begin ();
			    if (pos >= (int) escape.size ()) {
				auto real_loc = loc;
				real_loc.column += it;
				Error::occur (real_loc, ExternalError::get (UNDEFINED_ESCAPE));
			    }
				    
			    buf.write ((char) values [pos]);
			}
		    } else {
			auto real_loc = loc;
			real_loc.column += it;
			Error::occur (real_loc, ExternalError::get (UNTERMINATED_SEQUENCE));
		    }
		} else buf.write (content [it]);
		it ++;
	    }
	    return buf.str ();
	}
		
	uint UtfVisitor::convertChar (const lexing::Word & loc, const lexing::Word & content, int size) {
	    auto str =  escapeChar (loc, content.str, size == 32 ? Keys::U32 : Keys::U8);
	    if (size == 32) {
		std::vector <uint> utf_32 = utf8_to_utf32 (str);			
		if (utf_32.size () != 1) {		    
		    Ymir::Error::occur (loc, ExternalError::get (MALFORMED_CHAR), "c32", utf_32.size ());
		}
		return utf_32 [0];
	    } else if (size == 8) {
		if (str.length () != 1)
		    Ymir::Error::occur (loc, ExternalError::get (MALFORMED_CHAR), "c8", str.length ());
		return str [0] & 0b01111111;
	    }
		    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return 0;
	}
	
    }
}

