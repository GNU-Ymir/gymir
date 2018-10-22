#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace RangeUtils {

	Ymir::Tree InstAffect (Word locus, InfoType, syntax::Expression left, syntax::Expression right);	
	
	Ymir::Tree InstIn (Word locus, InfoType, syntax::Expression left, syntax::Expression right);

	Ymir::Tree InstFst (Word locus, InfoType, syntax::Expression left, syntax::Expression);

	Ymir::Tree InstScd (Word locus, InfoType, syntax::Expression left, syntax::Expression);

	Ymir::Tree InstStep (Word locus, InfoType, syntax::Expression left, syntax::Expression);
	
	Ymir::Tree InstCast (Word, InfoType, syntax::Expression elem, syntax::Expression);

	Ymir::Tree InstApply (Word word, InfoType, std::vector <syntax::Var> & vars, syntax::Block bl, syntax::Expression expr);
	
	Ymir::Tree InstAddr (Word locus, InfoType, syntax::Expression elem, syntax::Expression);
	
    }

}
