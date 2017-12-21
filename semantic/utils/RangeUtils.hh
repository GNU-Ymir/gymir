#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace RangeUtils {

	Ymir::Tree InstAffect (Word locus, InfoType, syntax::Expression left, syntax::Expression right);	
	
	Ymir::Tree InstIn (Word locus, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstFst (Word locus, InfoType, syntax::Expression left);

	Ymir::Tree InstScd (Word locus, InfoType, syntax::Expression left);		

	Ymir::Tree InstCast (Word, InfoType, syntax::Expression elem, syntax::Expression);
	
    }

}
