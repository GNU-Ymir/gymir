#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace PtrUtils {

	Ymir::Tree InstAffect (Word locus, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstCast (Word locus, syntax::Expression elem, syntax::Expression typeExpr);
	
    }

}
