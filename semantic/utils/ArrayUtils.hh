#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace ArrayUtils {

	Ymir::Tree InstAffect (Word locus, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstAffectNull (Word locus, InfoType, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstPtr (Word locus, InfoType, syntax::Expression left);

	Ymir::Tree InstAccessInt (Word word, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstIs (Word word, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstNotIs (Word word, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstLen (Word word, InfoType, syntax::Expression elem, syntax::Expression);

	Ymir::Tree InstPtr (Word word, InfoType, syntax::Expression elem, syntax::Expression);

	Ymir::Tree InstAddr (Word word, InfoType, syntax::Expression elem, syntax::Expression);

	Ymir::Tree InstConcat (Word word, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstConcatAff (Word word, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstToArray (Word word, InfoType, syntax::Expression elem, syntax::Expression type);

	Ymir::Tree InstToString (Word word, InfoType type, syntax::Expression elem, syntax::Expression);
	
	Ymir::Tree InstApply (Word word, InfoType, std::vector <syntax::Var> & vars, syntax::Block bl, syntax::Expression expr);

	Ymir::Tree InstCall (Word loc, semantic::InfoType ret, syntax::Expression, syntax::Expression paramsExp, ApplicationScore);
	
	Ymir::Tree InstInit (Word locus, semantic::InfoType type, syntax::Expression);

    }

}
