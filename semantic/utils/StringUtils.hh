#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace StringUtils {

	Ymir::Tree InstAff (Word locus, syntax::Expression left, syntax::Expression right);	

	Ymir::Tree InstPtr (Word locus, syntax::Expression left);
	
    }

}
