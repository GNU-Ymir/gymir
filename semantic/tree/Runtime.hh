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
	static std::string TUPLE_INFO;
	static std::string AGGREGATE_INFO;
	static std::string BOOL_INFO;
	static std::string CHAR_INFO;
	static std::string FLOAT_INFO;
	static std::string DOUBLE_INFO;
		
	static std::string ARRAY_INFO_STATIC;
	static std::string ARRAY_INFO_DYNAMIC;

	static std::string DELEGATE_INFO;
	static std::string FUNC_PTR_INFO;

	static std::string PTR_INFO;
	static std::string RANGE_INFO;
	static std::string REF_INFO;
	
	static std::string TYPE_INFO_MODULE;
	static std::string TYPE_INFO_SUFFIX;


    };
       
}
