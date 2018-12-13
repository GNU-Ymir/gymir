#pragma once
#include "Error.hh"

namespace Ymir {

    namespace Languages {
	using namespace Private;
	
	const char** fr () {
	    static const char* phrases [Ymir::Private::LAST_ERROR];
	    static bool init = false;
	    if (!init) {
		phrases [SyntaxError] = "[%] attendues, mais %%% trouvé";
		phrases [SyntaxError2] = "%%% inattendue";
		phrases [SyntaxErrorFor] = "%%% inattendue pour l'expression %%%";
		phrases [EscapeChar] = "caractère d'échappement inconnu";
		phrases [NotATemplate] = "L'élément '%%%' n'est pas un template";
		phrases [NotATemplate2] = "Aucune spécialisation de template pour '%%%' avec '%%%'";
		phrases [EndOfFile] = "Fin de fichier inattendue";
		phrases [Unterminated] = "caractère de fin de chaîne '%%%' manquant";
		phrases [TakeAType] = "Prend un type en template";
		phrases [TemplateSpecialisation] = "La specialisation de template fonctionne avec '%%%'";
		phrases [TemplateCreation] = "Création de template";
		phrases [And] = "Et";
		phrases [Here] = "Ici";
		phrases [MultipleLoopName] = "L'identifiant de boucle '%%%' est déjà définis";
		phrases [ShadowingVar] = "'%%%' est déjà définis";
		phrases [RecursiveExpansion] = "Nombre maximum d'expansions récursives atteint";		
		phrases [UnknownType] = "Le type '%%%' n'existe pas";
		phrases [UndefVar] = "Identifiant '%%%' inconnu";
		phrases [UndefVar2] = "Identifiant '%%%' inconnu, peut être '%%%'";
		phrases [UninitVar] = "Utilisation d'une variable sans type, '%%%'";
		phrases [UseAsVar] = "'%%%' n'identifie pas une variable";
		phrases [UseAsType] = "'%%%' n'identifie pas un type";
		phrases [UndefinedOpMult] = "L'operateur '%%%%' est indéfinis pour le type '%%%' et (%)";
		phrases [UndefinedOp] = "L'operateur '%%%' entre les types '%%%' et '%%%' n'existe pas";
		phrases [UndefinedOpUnary] = "L'operateur '%%%' pour le type '%%%' n'existe pas";
		phrases [BreakOutSide] = "Break en dehors d'un élements breakable";
		phrases [BreakRefUndef] = "L'identifiant de boucle '%%%' n'existe pas";
		phrases [UndefinedAttr] = "Pas de propriété '%%%' pour le type '%%%'";
		phrases [TemplateInferType] = "Réference vers un type de retour déduis pour l'appel";
		phrases [TemplateInferTypeNote] = "Type déduis de la fonction";
		phrases [IncompatibleTypes] = "Types incompatibles '%%%' et '%%%'";
		phrases [ConstNoInit] = "Declaration constante '%%%' sans valeur d'initialisation";
		
		phrases [NotLValue] = "'%%%' n'est pas une lvalue";
		phrases [NoValueNonVoid] = "retour de fonction sans valeur dans une fonction ne retournant pas void";
		phrases [ReturnVoid] = "retour d'un élèment de type '%%%'";
		phrases [UnreachableStmt] = "Instruction inatteigable";
		phrases [MissingReturn] = "Il manque une instruction de retour pour la fonction '%%%' qui retourne '%%%'";
		phrases [ModuleDontExist] = "Le fichier source %%%.yr n'existe pas";
		phrases [ImportError] = "Lors de l'importation de module";
		phrases [NotImmutable] = "La valeur ne peut être connu à la compilation";
		init = true;		
	    } 
	    return phrases;
	}

	const char** en () {
	    static const char* phrases [Ymir::Private::LAST_ERROR];
	    static bool init = false;
	    if (!init) {
		phrases [SyntaxError] = "[%] expected, when %%% found";
		phrases [SyntaxError2] = "%%% unexpected";
		phrases [SyntaxErrorFor] = "%%% unexpected when analysing expression %%%";
		phrases [EscapeChar] = "unknown escape character";
		phrases [NotATemplate] = "'%%%' element is not a template";
		phrases [NotATemplate2] = "There is not a valid template specialisation for '%%%' with '%%%'";
		phrases [EndOfFile] = "unexpected end of file";
		phrases [Unterminated] = "missing terminating '%%%' character";
		phrases [TakeAType] = "take a type as template argument";
		phrases [TemplateSpecialisation] = "Template specialisation works with both";
		phrases [TemplateCreation] = "Template creation";
		phrases [TemplateCreationWith] = "Template creation with %";
		phrases [TemplateCreation2] = "From first template creation";
		phrases [And] = "And";
		phrases [Here] = "Here";
		phrases [MultipleLoopName] = "Loop identifier '%%%' is already used";
		phrases [MainPrototype] = "Function main parameters must be main () or main ([string])";

		phrases [MainPrototypeStand] = "Function main parameters in standalone context must be main () or main (i32, p!(p!char))";
		phrases [MainInModule] = "Function main can't be declared in a sub module";
		phrases [ShadowingVar] = "Identifier '%%%' is already used";
		phrases [UnknownType] = "There is no type called '%%%'";
		phrases [RecursiveExpansion] = "Maximum number of recursive expansions reached";
		phrases [RecursiveInline] = "Recursive inline is impossible";
		phrases [UndefVar] = "Undefined identifier '%%%'";
		phrases [UndefVar2] = "Undefined identifier '%%%', maybe '%%%' was meant";
		phrases [UninitVar] = "Variable '%%%' does not have type";
		phrases [UseAsVar] = "'%%%' does not identify a variable";
		phrases [UseAsTrait] = "'%%%' does not identify a trait";
		phrases [UseAsType] = "'%%%' does not identify a type";
		phrases [CannotBeVoid] = "'%%%' can't be of type void";
		phrases [UndefinedOpMult] = "Operator '%%%%' is undefined for type '%%%' and (%)";
		phrases [UndefinedOp] = "Operator '%%%' between types '%%%' and '%%%' doesn't exist";
		phrases [UndefinedOpUnary] = "Operator '%%%' for type '%%%' doesn't exist";
		phrases [IncompatibleTypes] = "Incompatible types '%%%' and '%%%'";
		phrases [BreakOutSide] = "Break outside of a breakable scope";
		phrases [UndefinedAttr] = "No property '%%%' for type '%%%'";
		phrases [TemplateInferType] = "Forward reference to inferred return type of function call";

		phrases [RecursiveNoSize] = "Field %%% has no size because of forward reference";
		
		phrases [TemplateInferTypeNote] = "Inferred from";
		phrases [BreakRefUndef] = "Loop identifier '%%%' doesn't exist";
		phrases [ConstNoInit] = "Const declaration '%%%' with no initial value";
		phrases [StaticNoInit] = "Static declaration '%%%' with no initial value";
		phrases [ImmutNoInit] = "Immutable declaration '%%%' with no initial value";
		phrases [NotLValue] = "'%%%' is not a lvalue";
		phrases [NoValueNonVoid] = "Function exit without value in a non void function";

		phrases [ReturnVoid] = "Cannot return a '%%%' element";
		phrases [UnreachableStmt] = "Unreachable statement";
		phrases [LineInstruction] = "Use '{ }' for an empty statement, not a ';'";
		phrases [MissingReturn] = "Missing return statement for function '%%%' expecting a value of type '%%%'";		
		phrases [ModuleDontExist] = "Source file %%%.yr doesn't exist";		
		phrases [ImportError] = "Importation failed";
		phrases [PermissionDenied] = "Cannot open file named %%%, permission denied";
		phrases [NotImmutable] = "The value cannot be read at compile time";
		phrases [ModuleNotFirst] = "Global module declaration must be the first declaration";
		phrases [ModuleDontMatch] = "Module %%% must be placed in file %%%.yr";
		phrases [ShadowingVar2] = "Cannot override '%%%' with a different acces property (%%%)";
		phrases [AssertFailure] = "Assert failure : %";
		phrases [AssertFailure2] = "Assert failure";
		phrases [ScopeExitEnd] = "Scope guard declaration after scope exit";
		phrases [Overflow] = "Overflow capacity for type %%%";
		phrases [OverflowArray] = "Out of range %%% > %%%";
		phrases [MoreErrors] = "... (and % more, -v to show)\n";
		phrases [LabelingImmutableFor] = "Cannot label an immutable for";
		phrases [NotAMacro] = "%%% does not identify a macro";
		phrases [MacroResolution] = "Macro resolution %%% failed";
		phrases [NoLet] = "Malformed statement %%%";
		phrases [UndefUda] = "Undefined attribute %%% for %%%";
		phrases [UnrefInSafe] = "Cannot unref pointer in @safe context";
		phrases [ThrowInSafe] = "Cannot throw in @safe context"; 
		phrases [CallFuncPtrInSafe] = "Cannot call function pointer in @safe context";
		phrases [OutOfRange] = "The index is out of range %%% < %%%";
		phrases [DynamicAccess] = "Cannot make a dynamic access in @safe context";
		phrases [AllocationInSafe] = "Cannot allocate memory in @safe context";
		phrases [CallUnsafeInSafe] = "Cannot call unsafe context in @safe context";
		phrases [AddrLocalVar] = "Escaping reference to local variable %%%";
		phrases [UnPureExternC] = "Extern C function must be pure";
		phrases [ImplicitModule] = "Cannot use implicitly declared module";
		phrases [MultiDestr] = "A type cannot have multiple destructors";
		phrases [MultiStaticInit] = "Multiple static constructors";
		phrases [NeedAllType] = "All type must be known in constructor";
		phrases [StaticMethodInit] = "The keyword %init% is reserved for constructors; Note that constructors are always named %self%";
		phrases [StaticMethodOver] = "Cannot override a non-virtual function";
		phrases [CannotImpl] = "Aggregate type cannot implement type %%%";
		phrases [CannotOverride] = "Aggregate type cannot override type %%%";
		phrases [MustCallSuper] = "Must call the super constructor of type %%% first";
		phrases [ImplicitOverride] = "Cannot implicitly override method";
		phrases [NoOverride] = "Method %%% does not override anything";
		phrases [SelfAlwaysInfered] = "self parameter's type is always infered";
		phrases [SelfAlwaysRef] = "self parameter's type is always a reference";
		phrases [AmbiguousAccess] = "Ambiguous access of field for type %%%";
		phrases [PrivateMemberWithinThisContext] = "type %%% member %%% is private within current context";
		phrases [OverTemplateMethodDuo] = "Cannot override a non-virtual template method";
		phrases [OverTemplateMethod] = "Template method cannot override a super method";
		phrases [OverPrivateMethod] = "Private method are not virtual and cannot override";
		phrases [ClosureVarLifetime] = "Closure var '%%%' does not live long enough";
		phrases [UndefinedScopeEvent] = "'%%%' is not a scope event";
		phrases [VersionOnlyGlob] = "version conditions can only be declared at module scope";
		phrases [AggMatchOnlyNamed] = "match on object requires the use of named expression";
		init = true;
	    } 
	    return phrases;
	}
	
	const char** getLanguage (Language ln) {
	    switch (ln) {
	    case FR : return fr ();
	    case EN : return en ();
	    }
	    return NULL;
	}
	


    }

	

}
