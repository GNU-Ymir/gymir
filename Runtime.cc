#include <ymir/semantic/tree/Runtime.hh>

namespace Ymir {

    std::string Runtime::EXC_RETHROW = "_y_exc_rethrow";
    std::string Runtime::EXC_THROW = "_y_exc_throw";
    std::string Runtime::EXC_PUSH = "_y_exc_push";
    std::string Runtime::EXC_CHECK_TYPE = "_y_exc_check_type";
    std::string Runtime::SETJMP = "setjmp";

    int Runtime::JMP_BUF_SIZE = 200;

    std::string Runtime::STRUCT_INFO = "Struct_info";

}
