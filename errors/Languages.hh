#pragma once
#include "Error.hh"

namespace Ymir {

    namespace Languages {
	using namespace Private;
	
	const char** fr () {
	    static const char* phrases [Ymir::Private::LAST_ERROR];
	    static bool init = false;
	    if (!init) {
		phrases [SyntaxError] = "[%s] attendues, mais %s%s%s trouvé";
		phrases [SyntaxError2] = "%s%s%s inattendue";
		phrases [EscapeChar] = "caractère d'échappement inconnu";
		phrases [NotATemplate] = "L'élément '%s%s%s' n'est pas un template";
		phrases [EndOfFile] = "Fin de fichier inattendue";
		phrases [Unterminated] = "caractère de fin de chaîne '%s%s%s' manquant";
		phrases [TakeAType] = "Prend un type en template";
		phrases [TemplateSpecialisation] = "La specialisation de template fonctionne avec '%s%s%s'";
		phrases [TemplateCreation] = "Création de template";
		phrases [And] = "Et";
		phrases [Here] = "Ici";
		phrases [MultipleLoopName] = "L'identifiant de boucle '%s%s%s' est déjà définis";
		phrases [ShadowingVar] = "'%s%s%s' est déjà définis";
		phrases [RecursiveExpansion] = "Nombre maximum d'expansions récursives atteint";		
		phrases [UnknownType] = "Le type '%s%s%s' n'existe pas";
		phrases [UndefVar] = "Identifiant '%s%s%s' inconnu";
		phrases [UndefVar2] = "Identifiant '%s%s%s' inconnu, peut être '%s%s%s'";
		phrases [UninitVar] = "Utilisation d'une variable sans type, '%s%s%s'";
		phrases [UseAsVar] = "'%s%s%s' n'identifie pas une variable";
		phrases [UndefinedOpMult] = "L'operateur '%s%s%s%s' est indéfinis pour le type '%s%s%s' et (%s)";
		phrases [UndefinedOp] = "L'operateur '%s%s%s' entre les types '%s%s%s' et '%s%s%s' n'existe pas";
		phrases [UndefinedOpUnary] = "L'operateur '%s%s%s' pour le type '%s%s%s' n'existe pas";
		phrases [BreakOutSide] = "Break en dehors d'un élements breakable";
		phrases [BreakRefUndef] = "L'identifiant de boucle '%s%s%s' n'existe pas";
		phrases [UndefinedAttr] = "Pas de propriété '%s%s%s' pour le type '%s%s%s'";
		phrases [TemplateInferType] = "Réference vers un type de retour déduis pour l'appel";
		phrases [TemplateInferTypeNote] = "Type déduis de la fonction";
		phrases [IncompatibleTypes] = "Types incompatibles '%s%s%s' et '%s%s%s'";
		init = true;
	    } 
	    return phrases;
	}

	const char** en () {
	    static const char* phrases [Ymir::Private::LAST_ERROR];
	    static bool init = false;
	    if (!init) {
		phrases [SyntaxError] = "[%s] expected, when %s%s%s found";
		phrases [SyntaxError2] = "%s%s%s unexpected";
		phrases [EscapeChar] = "unknown escape character";
		phrases [NotATemplate] = "'%s%s%s' element is not a template";
		phrases [EndOfFile] = "unexpected end of file";
		phrases [Unterminated] = "missing terminating '%s%s%s' character";
		phrases [TakeAType] = "take a type as template argument";
		phrases [TemplateSpecialisation] = "Template specialisation works with both";
		phrases [TemplateCreation] = "Template creation";
		phrases [And] = "And";
		phrases [Here] = "Here";
		phrases [MultipleLoopName] = "Loop identifier '%s%s%s' is already used";

		phrases [ShadowingVar] = "Identifier '%s%s%s' is already used";
		phrases [UnknownType] = "There is no type called '%s%s%s'";
		phrases [RecursiveExpansion] = "Maximum number of recursive expansions reached";
		phrases [UndefVar] = "Undefined identifier '%s%s%s'";
		phrases [UndefVar2] = "Undefined identifier '%s%s%s', maybe '%s%s%s' was meant";
		phrases [UninitVar] = "Variable '%s%s%s' has no type";
		phrases [UseAsVar] = "'%s%s%s' do not identify a variable";
		phrases [UndefinedOpMult] = "Operator '%s%s%s%s' is undefined for type '%s%s%s' and (%s)";
		phrases [UndefinedOp] = "Operator '%s%s%s' between types '%s%s%s' and '%s%s%s' doesn't exist";
		phrases [UndefinedOpUnary] = "Operator '%s%s%s' for type '%s%s%s' doesn't exist";
		phrases [IncompatibleTypes] = "Incompatible types '%s%s%s' and '%s%s%s'";
		phrases [BreakOutSide] = "Break outside of a breakable scope";
		phrases [UndefinedAttr] = "No property '%s%s%s' for type '%s%s%s'";
		phrases [TemplateInferType] = "Forward reference to inferred return type of function call";
		phrases [TemplateInferTypeNote] = "Inferred from";
		phrases [BreakRefUndef] = "Loop identifier '%s%s%s' doesn't exist";
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
