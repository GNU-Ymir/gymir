#pragma once

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include "../syntax/Word.hh"
#include <cmath>
#include <ymir/utils/OutBuffer.hh>
#include <setjmp.h>

#define INCLUDE_STRING

#define TRY if( !setjmp(Ymir::Error::ex_buf__) )
#define CATCH else 
#define THROW longjmp(Ymir::Error::ex_buf__, 1)

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
	    TemplateCreationWith,
	    TemplateCreation2,
	    MoreErrors,
	    And,
	    Here,
	    RecursiveExpansion,
	    RecursiveNoSize,
	    RecursiveInline, 
	    MultipleLoopName,
	    ShadowingVar,
	    ShadowingVar2,
	    UnknownType,
	    UndefVar,
	    UndefVar2,
	    UndefUda,
	    UninitVar,
	    UndefinedOp,
	    UndefinedOpUnary,
	    UndefinedOpMult,
	    UseAsVar,
	    UseAsType,
	    UseAsTrait,
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
	    MainPrototypeStand,
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
	    NoLet,
	    UnrefInSafe,
	    ThrowInSafe,
	    CallFuncPtrInSafe,
	    OutOfRange,
	    DynamicAccess,
	    AllocationInSafe,
	    CallUnsafeInSafe,
	    LineInstruction,
	    CannotBeVoid,
	    AddrLocalVar,
	    UnPureExternC,
	    ImplicitModule,
	    MultiDestr,
	    MultiScopeFailure,
	    MultiStaticInit,
	    NeedAllType,
	    StaticMethodInit,
	    StaticMethodOver,
	    CannotImpl,
	    CannotOverride,
	    AmbiguousAccess,
	    MustCallSuper,
	    ImplicitOverride,
	    NoOverride,
	    SelfAlwaysInfered,
	    SelfAlwaysRef,
	    PrivateMemberWithinThisContext,
	    OverTemplateMethod,
	    OverTemplateMethodDuo,
	    OverPrivateMethod,
	    ClosureVarLifetime,
	    UndefinedScopeEvent,
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
	static jmp_buf ex_buf__;
	
	static void mainPrototype (const Word&);

	static void mainInModule (const Word&);

	static void moduleNotFirst (const Word &);

	static void moduleDontMatch (const Word &);
	
	static void notATemplate (const Word&);

	static void notATemplate (const Word&, std::vector <syntax::Expression>&, std::string);

	static void notImmutable (const Word &, semantic::Symbol);

	static void assertFailure (const Word &, const char* msg);

	static void assertFailure (const Word &);
	
	static void takeATypeAsTemplate (const Word&);

	static void unrefInSafe (const Word&);

	static void throwInSafe (const Word&);
		
	static void callFuncPtrInSafe (const Word&);

	static void returnLocalAddr (const Word&, const Word&);
	
	static void outOfRange (const Word&, ulong size, ulong index);

	static void dynamicAccess (const Word&);

	static void allocationInSafe (const Word&);

	static void callUnsafeInSafe (const Word&);

	static void unpureExternC (const Word&);
	
	static void syntaxError (const Word&);

	static void syntaxErrorFor (const Word&, const Word&);

	static void syntaxErrorFor (const Word&);

	static void syntaxError (const Word&, const char*);

	static void syntaxError (const Word&, const Word&);

	static void escapeError (const Word&);

	static void endOfFile (const Word&);

	static void unterminated (const Word&);

	static void overflow (const Word&, std::string);

	static void overflowArray (const Word&, ulong index, ulong len);

	static void templateSpecialisation (const Word&, const Word&);
	
	static void templateCreation (const Word&);

	static void templateCreation (const Word&, std::string tmps);

	static void templateCreation2 (const Word&, ulong nb);

	static void moduleDontExist (const Word&, const Word&);

	static void importError (const Word&);

	static void permissionDenied (std::string&);
	
	static void recursiveExpansion (const Word&);

	static void recursiveInlining (const Word&);

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

	static void noLet (const Word&);
	
	static void undefVar (const Word&, semantic::Symbol);

	static void undefAttr (const Word&, semantic::Symbol, syntax::Var);

	static void undefUda (const Word&, const Word&);
	
	static void uninitVar (const Word&, const Word&);

	static void implicitModule (const Word&);

	static void undefinedScopeEvent (const Word&);

	static void useAsVar (const Word&, semantic::Symbol);

	static void useAsTrait (const Word&);

	static void cannotBeVoid (const Word&);
	
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

	static void incompatibleTypes (const Word&, std::string, std::string);

	static void templateInferType (const Word&, const Word&);

	static void returnVoid (const Word&, semantic::Symbol);

	static void missingReturn (const Word&, semantic::Symbol);

	static void closureVarLifetime (const Word&, semantic::Symbol);

	static void here (const Word&);
	
	static void unreachableStmt (const Word&);

	static void scopeExitEnd (const Word&);
	
	static void unreachableStmtWarn (const Word&);

	static void lineInstructionWarn (const Word&);
	
	static void noValueNonVoidFunction (const Word&);

	static void multipleDestr (const Word &);
	
	static void multipleStaticInit (const Word &, const Word&);

	static void needAllTypeConstr (const Word&);

	static void staticMethodInit (const Word&);

	static void staticMethodOver (const Word&);

	static void cannotImpl (const Word&, semantic::InfoType);

	static void cannotOverride (const Word&, semantic::InfoType);
	
	static void mustCallSuperConstructor (const Word&, semantic::InfoType);

	static void implicitOverride (const Word&, const Word&);

	static void overTemplateMethod (const Word&);
	
	static void overTemplateMethod (const Word&, const Word&);
	
	static void overPrivateMethod (const Word&, const Word&);

	static void overPrivateMethod (const Word&);

	static void noOverride (const Word&);

	static void selfAlwaysInfered (const Word&);

	static void selfAlwaysRef (const Word&);
	
	static void privateMemberWithinThisContext (const std::string & str, const Word&);
	
	static void ambiguousAccess (const Word&, const Word&, semantic::InfoType);
	
	static void activeError (bool);	
		
	static std::vector <ErrorMsg>& caught ();

	static void assert (const char* format);
	
	template <typename ... TArgs>
	static void assert (const char * format_, TArgs ... args) {
	    return __instance__.assert_ (format_, args...);
	}

	template <typename ... TArgs>
	static void fail (const char * format_, TArgs ... args) {	    
	    //fatal_error (UNKNOWN_LOCATION, format_, args...);
	    fprintf (stderr, format_, args...);
	    THROW;
	}

	template <typename ... TArgs>
	static void end (const char * format_, TArgs ... args) {	    
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
	    fprintf (stderr, "%s", aux.c_str ());
	    THROW;
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
	    fprintf (stderr, "%s", aux.c_str ());
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
	}

	template <typename ... TArgs>
	static void note (const Word& word, const char * format_, TArgs ... args) {
	    __instance__.note_ (word, format_, args...);
	}
	
	template <typename ... TArgs>
	void assert_ (const char * format_, TArgs ... args) {	    
	    fprintf (stderr, "%sAssert%s : %s\n", RED, RESET, format (format_, args...).c_str ());
	    raise (SIGABRT);
	}	

	static bool isEnable ();	    	
	
    private:

	static std::vector <bool> __isEnable__;
	static std::vector <ErrorMsg> __caught__;
	static Error __instance__;
	
    };
    
}
