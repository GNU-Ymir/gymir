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
	    NotATemplate2,
	    TakeAType,
	    SyntaxError,
	    SyntaxErrorFor,
	    SyntaxError2,
	    EscapeChar,
	    EndOfFile,
	    Unterminated,
	    TemplateSpecialisation,
	    TemplateCreation,
	    TemplateCreation2,
	    MoreErrors,
	    And,
	    Here,
	    RecursiveExpansion,
	    RecursiveNoSize,
	    MultipleLoopName,
	    ShadowingVar,
	    ShadowingVar2,
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
	    StaticNoInit,
	    ImmutNoInit,
	    NotLValue,
	    NoValueNonVoid,
	    ReturnVoid,
	    UnreachableStmt,
	    MissingReturn,
	    ModuleDontExist,
	    ImportError,
	    PermissionDenied,
	    NotImmutable,
	    MainPrototype,
	    MainInModule,
	    ModuleNotFirst,
	    ModuleDontMatch,
	    AssertFailure,
	    AssertFailure2,
	    ScopeExitEnd,
	    Overflow,
	    OverflowArray,
	    LabelingImmutableFor,
	    NotAMacro,
	    MacroResolution,
	    LAST_ERROR
	};

	const char * getString (ErrorType, Language ln = EN);
    };

    
    std::string addLine (std::string, const Word& word);    
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

	static void mainPrototype (const Word&);

	static void mainInModule (const Word&);

	static void moduleNotFirst (const Word &);

	static void moduleDontMatch (const Word &);
	
	static void notATemplate (const Word&);

	static void notATemplate (const Word&, std::vector <syntax::Expression>&);

	static void notImmutable (const Word &, semantic::Symbol);

	static void assertFailure (const Word &, const char* msg);

	static void assertFailure (const Word &);
	
	static void takeATypeAsTemplate (const Word&);

	static void syntaxError (const Word&);

	static void syntaxErrorFor (const Word&, const Word&);

	static void syntaxError (const Word&, const char*);

	static void syntaxError (const Word&, const Word&);

	static void escapeError (const Word&);

	static void endOfFile ();

	static void unterminated (const Word&);

	static void overflow (const Word&, std::string);

	static void overflowArray (const Word&, ulong index, ulong len);

	static void templateSpecialisation (const Word&, const Word&);
	
	static void templateCreation (const Word&);

	static void templateCreation2 (const Word&, ulong nb);

	static void moduleDontExist (const Word&, const Word&);

	static void importError (const Word&);

	static void permissionDenied (std::string&);
	
	static void recursiveExpansion (const Word&);

	static void recursiveNoSize (const Word&);

	static void unknownType (const Word&);

	static void multipleLoopName (const Word&, const Word&);

	static void shadowingVar (const Word&, const Word&);

	static void shadowingVar (const Word&, const Word&, bool isPublic);

	static void constNoInit (const Word&);

	static void staticNoInit (const Word&);

	static void immutNoInit (const Word&);
	
	static void labelingImmutableFor (const Word&);

	static void notAMacro (const Word&);

	static void macroResolution (const Word&);

	static void notLValue (const Word&);
	
	static void undefVar (const Word&, semantic::Symbol);

	static void undefAttr (const Word&, semantic::Symbol, syntax::Var);
	
	static void uninitVar (const Word&);

	static void useAsVar (const Word&, semantic::Symbol);

	static void breakRefUndef (const Word&);

	static void breakOutSide (const Word&);
	
	static void useAsType (const Word&);
	
	static void undefinedOp (const Word&, const Word&, semantic::Symbol, syntax::ParamList);

	static void undefinedOp (const Word&, semantic::Symbol, syntax::ParamList);

	static void undefinedOp (const Word&, semantic::Symbol, semantic::Symbol);

	static void undefinedOp (const Word&, semantic::Symbol, semantic::InfoType);
	
	static void undefinedOp (const Word&, semantic::Symbol);

	static void incompatibleTypes (const Word&, semantic::Symbol, semantic::InfoType);

	static void incompatibleTypes (const Word&, semantic::Symbol, std::string);

	static void templateInferType (const Word&, const Word&);

	static void activeError (bool);	

	static void returnVoid (const Word&, semantic::Symbol);

	static void missingReturn (const Word&, semantic::Symbol);
	
	static void unreachableStmt (const Word&);

	static void scopeExitEnd (const Word&);
	
	static void unreachableStmtWarn (const Word&);
	
	static void noValueNonVoidFunction (const Word&);
	
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
	void fatal_ (const Word& word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (RED) + "Error" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    
	    fatal_error (UNKNOWN_LOCATION, "");
	}
		
	template <typename ... TArgs>
	static void fatal (const Word& word, const char * format_, TArgs ... args) {
	    __instance__.fatal_ (word, format_, args...);
	}

	template <typename ... TArgs>
	void append_ (const Word& word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (RED) + "Error" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    nb_errors ++;
	}

	template <typename ... TArgs>
	static void append (const Word& word, const char * format_, TArgs ... args) {
	    __instance__.append_ (word, format_, args...);
	}
	
	template <typename ... TArgs>
	void note_ (const Word& word, const char * format_, TArgs ... args) {
	    std::string aux = format (format_, args...);
	    aux = std::string (BLUE) + "Note" + std::string (RESET) + " : " + aux;
	    aux = addLine (aux, word);
	    printf ("%s", aux.c_str ());
	    
	    inform (UNKNOWN_LOCATION, "");
	}

	template <typename ... TArgs>
	static void note (const Word& word, const char * format_, TArgs ... args) {
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
