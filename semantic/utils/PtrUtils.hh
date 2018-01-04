#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace PtrUtils {

	Ymir::Tree InstAffect (Word locus, InfoType, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstCast (Word locus, InfoType, syntax::Expression elem, syntax::Expression typeExpr);

	Ymir::Tree InstUnref (Word locus, InfoType type, syntax::Expression elem);
	
    }

}
