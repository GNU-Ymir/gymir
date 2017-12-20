#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace ArrayUtils {

	Ymir::Tree InstAffect (Word locus, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstPtr (Word locus, syntax::Expression left);

	Ymir::Tree InstAccessInt (Word word, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstIs (Word word, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstNotIs (Word word, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstLen (Word loc, syntax::Expression elem);

	Ymir::Tree InstPtr (Word loc, syntax::Expression elem);

	Ymir::Tree InstConcat (Word loc, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstConcatAff (Word loc, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstCastToArray (Word loc, syntax::Expression elem, syntax::Expression type);
	
    }

}
