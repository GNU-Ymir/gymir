#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace TupleUtils {

	Ymir::Tree InstAffect (Word locus, InfoType, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstCast (Word locus, InfoType, syntax::Expression elem, syntax::Expression typeExpr);

	Ymir::Tree InstGet (Word locus, InfoType type, syntax::Expression elem, syntax::Expression index);
	
    }

}
