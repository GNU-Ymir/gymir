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
     * \struct IFixed
     * The syntaxic node representation of a integer constante
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

	ulong convertU ();
	long convertS ();

	ulong convertUX ();
	long convertSX ();
	
    };

    typedef unsigned char ubyte;
    
    class IChar : public IExpression {
	ubyte code;

    public:

	IChar (Word word, ubyte code);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	char toChar ();
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;
	
	std::string prettyPrint () override;
    };

    class IFloat : public IExpression {

	std::string suite;
	std::string totale;
	FloatConst _type;
	
    public:

	IFloat (Word word);
	
	IFloat (Word word, std::string suite);
	
	IFloat (Word word, std::string suite, FloatConst type);

	IFloat (Word word, FloatConst type);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	void setValue (float val);

	void setValue (double val);

	std::string getValue ();
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;
	
    };


    class IString : public IExpression {

	std::string content;
	
    public :

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
        
    class IBool : public IExpression {

	bool value;
	
    public :
	IBool (Word token);
	
	bool& getValue ();

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

    class INull : public IExpression {
    public:

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

    class IIgnore : public IExpression {
    public:
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
