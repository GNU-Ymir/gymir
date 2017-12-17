#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace ArrayUtils {

	Ymir::Tree InstAffect (Word locus, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstPtr (Word locus, syntax::Expression left);

	Ymir::Tree InstAccessInt (Word word, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstIs (Word word, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstNotIs (Word word, syntax::Expression left, syntax::Expression right);
	
    }

}
