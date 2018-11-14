#pragma once

#include "Expression.hh"
#include "../syntax/Word.hh"
#include <cmath>


enum class FixedConst {
    BYTE, UBYTE,
    SHORT, USHORT,
    INT, UINT,
    LONG, ULONG
};

enum class FixedMode {
    HEXA, DECIMAL, BUILTINS    
};

enum class FloatConst {
    FLOAT,
    DOUBLE
};

namespace syntax {
    /**
     * \param ct a type
     * \return true if and only if the type has a representation for negative number
     */
    bool isSigned (FixedConst ct);

    /**
     * \return the name of the type ct
     */
    std::string name (FixedConst ct);

    /**
     * \return the simple name of the type ct
     */
    std::string sname (FixedConst ct);

    /**
     * \return the infotype name of the type ct
     */
    std::string infoname (FixedConst ct);
    
    /**
     * \struct IFixed
     * The syntaxic node representation of a integer constante
     * \verbatim
     fixed := ([0-9])+('_' [0-9])*
     \endverbatim
     */
    class IFixed : public IExpression {
	FixedConst _type;

	ulong _uvalue;
	ulong _value;
	FixedMode _mode;
	
    public:

	/**
	 * \param word the location of the constante
	 * \param type the type of the constante
	 */
	IFixed (Word word, FixedConst type);

	/**
	 * \param word the location of the constante
	 * \param type the type of the constante
	 * \param mode the encryption of the constante
	 */
	IFixed (Word word, FixedConst type, FixedMode mode);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;

	void setUValue (ulong val);

	void setValue (long val);
	
	void print (int nb = 0) override;

	static const char * id () {
	    return TYPEID (IFixed);
	}
	
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IFixed));
	    return ret;
	}
	
	std::string prettyPrint () override;

	/**
	 * \return the constante string without any '_'
	 */
	std::string removeUnder (); 
	
    private:

	/**
	 * \brief Convert the string value (read from syntax analyses) to ulong	   
	 */
	ulong convertU ();

	/**
	 * \brief Convert the string value (read from syntax analyses) to long	   
	*/
	long convertS ();

	/**
	 * \brief Convert the string value (read from syntax analyses) to ulong from hexadecimal encoding
	*/
	ulong convertUX ();

	/**
	 * \brief Convert the string value (read from syntax analyses) to long from hexadecimal encoding
	*/
	long convertSX ();
	
    };

    typedef unsigned char ubyte;

    /**
     * \struct IChar
     * The syntaxic node representation of a char constante
     * \verbatim
     char := '\'' inner_char '\''
     inner_char := [.] | ('\\' ('0' [0-8]+ | 'x' ([0-9]|[a-e]|[A-E])+ | escape_char))
     escape_char := ('a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v' | '\\' | '\'' | '"' | '?') 
     \endverbatim
     */
    class IChar : public IExpression {
	/** The value of the constante */
	ubyte _code;

    public:

	/**
	 * \param word the location of the constante
	 * \param code the value of the constante
	 */
	IChar (Word word, ubyte code);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	/**
	 * \brief Convert this constante to a char value
	 */
	char toChar ();
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;
	
	std::string prettyPrint () override;
    };

    /**
     * \struct IFloat
     * The syntaxic node representation of a floating point constante
     * \verbatim
     float := (((fixed)? '.' fixed) | (fixed '.' (fixed)?) ('f')?
     \endverbatim
     */
    class IFloat : public IExpression {

	/** The decimal part of this constante */
	std::string _suite;

	/** The value of the constante (including the decimal part) */
	std::string _totale;

	/** The type of constante (float, double) */
	FloatConst _type;
	
    public:

	/**
	 * \param word the location of the constante
	 */
	IFloat (Word word);

	/**
	 * \param word the location of the constante
	 * \param suite the decimal part of the constante
	 */
	IFloat (Word word, std::string suite);

	/**
	 * \param word the location of the constante
	 * \param suite the decimal part of the constante
	 * \param type the type of constante
	 */
	IFloat (Word word, std::string suite, FloatConst type);

	/**
	 * \param word the location of the constante
	 * \param type the type of the constante
	 */
	IFloat (Word word, FloatConst type);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	/**
	 * \brief Set the value of the the constante 
	 * \param val the new value
	 */
	void setValue (float val);


	/**
	 * \brief Set the value of the the constante 
	 * \param val the new value
	 */
	void setValue (double val);

	/**
	 * \return a string encoding of the constante
	 */
	std::string getValue ();
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;
	
    };

    /**
     * \struct IString
     * The syntaxic node representation of a string constante
     * \verbatim
     string := '\"' char* '\"'
     \endverbatim
     */
    class IString : public IExpression {
	
	/** The content of the string */
	std::string _content;
	
    public :

	/**
	 * \param word the location of the constante
	 * \param content the value
	 */
	IString (Word word, std::string content);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;	

	std::string getStr ();
	
	static const char * id () {
	    return TYPEID (IString);
	}
	
	std::vector <std::string> getIds () override;
	
    private:

	/**
	 * \return the short value of an escaping char in octal 
	 */
	static short getFromOc (std::string elem, ulong & index) {
	    auto fst = elem [index + 1];
	    index += 2;
	    if (fst < '0' || fst > '7') return -1;
	    int size = 1;
	    for (int i = index ; i < (int) elem.size (); i++) {
		if (elem [i] < '0' ||  elem [i] > '7') break;
		else size ++;
	    }
	    size = size > 5 ? 5 : size;

	    short total = 0;
	    int current = size - 1;	    
	    for (int i = index - 1 ; i < (int) (index + size - 1) ; i ++, current --) {
		total += pow (8, current) * (elem [i] - '0');
	    }

	    index += size - 1;
	    return total;
	}

	/**
	 * \return the short value of an escaping char in hexa
	 */
	static short getFromLX (std::string elem, ulong & index) {
	    index += 2;
	    int size = 0;
	    for (int i = index ; i < (int) elem.size (); i++) {
		if ((elem [i] < '0' ||  elem [i] > '9') &&
		    (elem [i] < 'a' || elem [i] > 'f')
		) break;
		else size ++;
	    }
	    
	    size = size > 2 ? 2 : size;
	    short total = 0;
	    int current = size - 1;	    
	    for (int i = index ; i < (int) (index + size) ; i ++, current --) {
		if (elem [i] >= 'a')
		    total += pow (16, current) * (elem [i] - 'a' + 10);
		else
		    total += pow (16, current) * (elem [i] - '0');
	    }
	    index += size;
	    return total;
	}

	/**
	 * \return the index where the the char <em>what</em> can be found
	 */
	static int find (const char* where, char what) {
	    int i = 0;
	    while (*where != '\0') {
		if (what == *where) return i;
		i ++;
		where++;
	    }
	    return -1;
	}

    public:

	/**
	 * \param current a string
	 * \param index the current index 
	 * \return the char encoded at index <em>index</em>, (including escaping representation)
	 */
	static short isChar (std::string &current, ulong &index) {
	    static char escape[] = {'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\'', '"', '?', '\0'};
	    static short values[] = {7, 8, 12, 10, 13, 9, 11, 92, 39, 34, 63};
	    if (current [index] != '\\') {
		index ++;
		return current [index - 1];
	    } else {
		if (index + 1 >= current.size ()) return -1;
		auto next = current [index + 1];
		auto p = find (escape, next);
		if (p != -1) {
		    index += 2;
		    return values [p];
		} else if (next == 'x') {
		    return getFromLX (current, index);		    
		} else return getFromOc (current, index);
	    }
	}	
	
    };
        
    /**
     * \struct IBool
     * The syntaxic node representation of a boolean constante     
     * \verbatim
     bool := 'true' | 'false'
     \endverbatim
     */
    class IBool : public IExpression {

	/** The value of the constante */
	bool _value;
	
    public :

	/**
	 * \param token the location of the constante
	 */
	IBool (Word token);
	
	bool& value ();

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Bool> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	}

	std::string prettyPrint () override;
    };    

    /**
     * \struct INull
     * The syntaxic node representation of a null constante
     * \verbatim
     null := 'null'
     \endverbatim
     */
    class INull : public IExpression {
    public:

	/**
	 * \param token the location of the constante
	 */
	INull (Word token) : IExpression (token) {
	}

	Expression expression () override;	
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Null> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	}

	std::string prettyPrint () override;
	
    };


    /**
     * \struct IIgnore
     * The syntaxic node representation of an ignore constante
     * \verbatim
     ignore := '_'
     \endverbatim
     */
    class IIgnore : public IExpression {
    public:
	
	/**
	 * \param token the location of the constante
	 */
	IIgnore (Word token)
	    : IExpression (token)
	{}

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
		
	static const char * id () {
	    return TYPEID (IIgnore);
	}
	
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IIgnore));
	    return ret;
	}

	void print (int nb = 0) override {
	    printf ("\n%*c<Ignore> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	}

	std::string prettyPrint () override;
    };
    
    typedef IFixed* Fixed;
    typedef IChar* Char;
    typedef IFloat* Float;
    typedef IString* String;
    typedef IBool* Bool;
    typedef INull* Null;
    typedef IIgnore* Ignore;
}
