#pragma once

#include <string>

namespace Ymir {

    struct Runtime {
	static std::string EXC_RETHROW;
	static std::string EXC_THROW;
	static std::string EXC_PUSH;
	static std::string EXC_CHECK_TYPE;
	static std::string SETJMP;       	

	static int JMP_BUF_SIZE;

	static std::string STRUCT_INFO;
    };
       
}
