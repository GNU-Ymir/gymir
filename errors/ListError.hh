#pragma once

#include <ymir/utils/Enum.hh>

namespace Ymir {

    /**
     * \enum ErrorCode
     * This enum is used for exception handling
     */
    enum class ErrorCode : int {
	INTERNAL = 0, // internal error, due to compiler problem
	EXTERNAL = 1, // external error, due to source code error
	FATAL = 2, // fatal external error
	FAIL = 3 // fail error, too many errors for example, the compilation must stop
    };    

    /**
       \enum ErrorType
       This enumaration lists all the error types 
       We cannot use C++ enum, as it does not allows to have string values
    */
    DECLARE_ENUM_HEADER (ExternalError, std::string,			         
			 SYNTAX_ERROR_AT, // syntax error a specific location
			 NO_SUCH_FILE,
			 COMPILATION_END
    );
    
    // 	    SYNTAX_ERROR_FOR = "%%% unexpected when analysing statement %%%",
    // 	    SYNTAX_ERROR_SIMPLE = "%%% unexpected",
    // 	    LAST_ERROR
    // 	    };
        
}
