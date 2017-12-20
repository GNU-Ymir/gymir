#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include "../syntax/Word.hh"
#include <cmath>
#include <ymir/utils/OutBuffer.hh>

#define INCLUDE_STRING

namespace semantic {
    class ISymbol;
    typedef ISymbol* Symbol;

    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {
    class IParamList;
    typedef IParamList* ParamList;

    class IVar;
    typedef IVar* Var;    
}

namespace Ymir {

    enum Language {
	FR,
	EN
    };
    
    namespace Private {
	enum ErrorType {
	    NotATemplate = 1,
	    TakeAType,
	    SyntaxError,
	    SyntaxError2,
	    EscapeChar,
	    EndOfFile,
	    Unterminated,
	    TemplateSpecialisation,
	    TemplateCreation,
	    And,
	    Here,
	    RecursiveExpansion,
	    MultipleLoopName,
	    ShadowingVar,
	    UnknownType,
	    UndefVar,
	    UndefVar2,
	    UninitVar,
	    UndefinedOp,
	    UndefinedOpUnary,
	    UndefinedOpMult,
	    UseAsVar,
	    UseAsType,
	    IncompatibleTypes,
	    BreakOutSide,
	    BreakRefUndef,
	    UndefinedAttr,
	    TemplateInferType,
	    TemplateInferTypeNote,
	    ConstNoInit,
	    NotLValue,
	    LAST_ERROR
	};

	const char * getString (ErrorType, Language ln = EN);
    };

    
    std::string addLine (std::string, Word word);    
    bool isVerbose ();
    
    template <typename ... T> 
    std::string format (std::string left, T... params) {
	OutBuffer buf;
	buf.writef (left.c_str (), params...);
	return buf.str ();
    }
    
    template <typename ... T> 
    std::string format (const char* left, T... params) {
	OutBuffer buf;
	buf.writef (left, params...);
	return buf.str ();
    }
      
    struct ErrorMsg {
	std::string msg;
	bool isFatal;
	bool isWarning;       	
    };
    
    struct Error {

	static const char*	RESET;	//= "\u001B[0m";
	static const char*	PURPLE;	//= "\u001B[1;35m";
	static const char*	BLUE;	//= "\u001B[1;36m";
	static const char*	YELLOW;	//= "\u001B[1;33m";
	static const char*	RED;	//= "\u001B[1;31m";
	static const char*	GREEN;	// = "\u001B[1;32m";
	static const char*	BOLD;	// = "\u001B[1;50m";
	
	static unsigned long nb_errors;// (0);

	static void notATemplate (Word);

	static void takeATypeAsTemplate (Word);

	static void syntaxError (Word);

	static void syntaxError (Word, const char*);

	static void syntaxError (Word, Word);

	static void escapeError (Word);

	static void endOfFile ();

	static void unterminated (Word);

	static void templateSpecialisation (Word, Word);
	
	static void templateCreation (Word);

	static void recursiveExpansion (Word);

	static void unknownType (Word);

	static void multipleLoopName (Word, Word);

	static void shadowingVar (Word, Word);

	static void constNoInit (Word);

	static void notLValue (Word);
	
	static void undefVar (Word, semantic::Symbol);

	static void undefAttr (Word, semantic::Symbol, syntax::Var);
	
	static void uninitVar (Word);

	static void useAsVar (Word, semantic::Symbol);

	static void breakRefUndef (Word);

	static void breakOutSide (Word);
	
	static void useAsType (Word);
	
	static void undefinedOp (Word, Word, semantic::Symbol, syntax::ParamList);

	static void undefinedOp (Word, semantic::Symbol, syntax::ParamList);

	static void undefinedOp (Word, semantic::Symbol, semantic::Symbol);

	static void undefinedOp (Word, semantic::Symbol, semantic::InfoType);
	
	static void undefinedOp (Word, semantic::Symbol);

	static void incompatibleTypes (Word, semantic::Symbol, semantic::InfoType);

	static void templateInferType (Word, Word);

	static void activeError (bool);	
	
	static std::vector <ErrorMsg>& caught ();

	static void assert (const char* format);
	
	template <typename ... TArgs>
	static void assert (const char * format_, TArgs ... args) {
	    return __instance__.assert_ (format_, args...);
	}

	template <typename ... TArgs>
	static void fail (const char * format_, TArgs ... args) {	    
	    fatal_error (UNKNOWN_LOCATION, format_, args...);
	}
	
	static bool thrown () {
	    return nb_errors != 0;
	}

	static Error instance () {
	    return __instance__;
	}	
	    
    private:
	
	template <typename ... TArgs>
	void fatal_ (Word word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (RED) + "Error" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    
	    fatal_error (UNKNOWN_LOCATION, "");
	}
		
	template <typename ... TArgs>
	static void fatal (Word word, const char * format_, TArgs ... args) {
	    __instance__.fatal_ (word, format_, args...);
	}

	template <typename ... TArgs>
	void append_ (Word word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (RED) + "Error" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    nb_errors ++;
	}

	template <typename ... TArgs>
	static void append (Word word, const char * format_, TArgs ... args) {
	    __instance__.append_ (word, format_, args...);
	}
	
	template <typename ... TArgs>
	void note_ (Word word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (BLUE) + "Note" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    
	    inform (UNKNOWN_LOCATION, "");
	}

	template <typename ... TArgs>
	static void note (Word word, const char * format_, TArgs ... args) {
	    __instance__.note_ (word, format_, args...);
	}
	
	template <typename ... TArgs>
	void assert_ (const char * format_, TArgs ... args) {	    
	    printf ("%sAssert%s : %s\n", RED, RESET, format (format_, args...).c_str ());
	    raise (SIGSEGV);
	}	
	
    private:

	static bool __isEnable__;
	static std::vector <ErrorMsg> __caught__;
	static Error __instance__;
	
    };
    
}
