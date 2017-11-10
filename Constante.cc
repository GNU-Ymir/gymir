#include "ast/Constante.hh"

namespace syntax {

    bool isSigned (FixedConst ct) {
	return ((int) ct) % 2 == 0;
    }
    
}
