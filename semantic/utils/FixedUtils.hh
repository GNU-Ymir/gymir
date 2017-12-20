#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace FixedUtils {

	Ymir::Tree InstAffect (Word locus, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree UnaryMinus (Word locus, syntax::Expression left);
	Ymir::Tree InstSSub (Word locus, syntax::Expression left);
	Ymir::Tree InstPPlus (Word locus, syntax::Expression left);

	Ymir::Tree InstCast (Word locus, syntax::Expression elem, syntax::Expression type);

	Ymir::Tree InstAddr (Word locus, syntax::Expression elem, syntax::Expression);
	Ymir::Tree InstReaff (Word locus, syntax::Expression elem, syntax::Expression);
	Ymir::Tree InstNormal (Word locus, syntax::Expression elem, syntax::Expression);
	Ymir::Tree InstNormalRight (Word locus, syntax::Expression elem, syntax::Expression);
	
    }

}
