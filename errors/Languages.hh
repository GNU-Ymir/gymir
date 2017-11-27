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
		phrases [ShadowingVar] = "L'identifiant de boucle '%s%s%s' est déjà définis";
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
