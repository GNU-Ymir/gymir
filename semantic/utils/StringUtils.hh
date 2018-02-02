#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace StringUtils {

	Ymir::Tree InstAff (Word locus, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstAddr (Word locus, InfoType, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstPtr (Word locus, InfoType, syntax::Expression left, syntax::Expression);

	Ymir::Tree InstLen (Word locus, InfoType, syntax::Expression left, syntax::Expression);	
	
	Ymir::Tree InstToString (Word locus, InfoType, syntax::Expression elem, syntax::Expression type);

	Ymir::Tree InstToArray (Word locus, InfoType, syntax::Expression elem, syntax::Expression type);

	Ymir::Tree InstConcat (Word locus, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstConcatAff (Word locus, InfoType, syntax::Expression left, syntax::Expression right);
	
	Ymir::Tree InstAccessInt (Word word, InfoType, syntax::Expression left, syntax::Expression right);	

    }

}
