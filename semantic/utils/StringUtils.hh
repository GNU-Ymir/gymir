#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace StringUtils {

	Ymir::Tree InstAff (Word locus, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstPtr (Word locus, syntax::Expression left);

	Ymir::Tree InstToString (Word locus, syntax::Expression elem, syntax::Expression type);

	Ymir::Tree InstConcat (Word locus, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstConcatAff (Word locus, syntax::Expression left, syntax::Expression right);
	
	Ymir::Tree InstAccessInt (Word word, syntax::Expression left, syntax::Expression right);	

    }

}
