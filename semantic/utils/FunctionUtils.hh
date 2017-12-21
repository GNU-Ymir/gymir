#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    namespace FunctionUtils {

	Ymir::Tree InstCall (Word locus, InfoType, ApplicationScore score, syntax::Expression left, std::vector <syntax::Expression> & right);	

    }

}
