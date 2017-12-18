#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace FixedUtils {

	Ymir::Tree InstAffInt (Word locus, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree UnaryMinus (Word locus, syntax::Expression left);
	
    }

}
