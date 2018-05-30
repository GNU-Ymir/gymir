#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace StructUtils {
	Ymir::Tree InstCall (Word, InfoType, syntax::Expression, syntax::Expression paramsExp);

	Ymir::Tree InstCallUnion (Word, InfoType, syntax::Expression, syntax::Expression);

	Ymir::Tree InstCast (Word, InfoType, syntax::Expression, syntax::Expression);

	Ymir::Tree InstCastTuple (Word, InfoType, syntax::Expression, syntax::Expression);

	Ymir::Tree InstInit (Word, InfoType, syntax::Expression, syntax::Expression);

	Ymir::Tree InstGet (Word, InfoType, syntax::Expression, syntax::Expression);

	Ymir::Tree InstAffect (Word, InfoType, syntax::Expression, syntax::Expression);

	Ymir::Tree InstAddr (Word, InfoType, syntax::Expression, syntax::Expression);

	Ymir::Tree InstSizeOf (Word, InfoType, syntax::Expression);

	Ymir::Tree InstSizeOfCst (Word, InfoType, syntax::Expression);
	
    }
    
}
