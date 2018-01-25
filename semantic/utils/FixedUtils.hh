#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace FixedUtils {

	Ymir::Tree InstAffect (Word locus, InfoType, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree UnaryMinus (Word locus, InfoType, syntax::Expression left);
	Ymir::Tree InstSSub (Word locus, InfoType, syntax::Expression left);
	Ymir::Tree InstPPlus (Word locus, InfoType, syntax::Expression left);

	Ymir::Tree InstCast (Word locus, InfoType, syntax::Expression elem, syntax::Expression type);

	Ymir::Tree InstAddr (Word locus, InfoType, syntax::Expression elem, syntax::Expression);
	Ymir::Tree InstReaff (Word locus, InfoType, syntax::Expression elem, syntax::Expression);
	Ymir::Tree InstNormal (Word locus, InfoType, syntax::Expression elem, syntax::Expression);
	Ymir::Tree InstNormalRight (Word locus, InfoType, syntax::Expression elem, syntax::Expression);

	Ymir::Tree InstTest (Word locus, InfoType, syntax::Expression elem, syntax::Expression);
	Ymir::Tree InstTestRight (Word locus, InfoType, syntax::Expression elem, syntax::Expression);

	Ymir::Tree InstRange (Word locus, InfoType, syntax::Expression elem, syntax::Expression);
	Ymir::Tree InstRangeRight (Word locus, InfoType, syntax::Expression elem, syntax::Expression);

	Ymir::Tree InstSizeOf (Word, InfoType type, syntax::Expression);
	
    }

}
