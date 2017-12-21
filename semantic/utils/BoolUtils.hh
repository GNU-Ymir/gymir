#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace BoolUtils {

	Ymir::Tree InstAffect (Word locus, InfoType, syntax::Expression left, syntax::Expression right);	

    }

}
