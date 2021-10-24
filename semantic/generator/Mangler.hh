#pragma once

#include <ymir/semantic/generator/_.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Mangler
	 * This class is used to mangle names in order to have uniq symbol for each declarations
	 */
	class Mangler {

	    static std::string YMIR_PREFIX;

	    static std::string YMIR_FUNCTION;

	    static std::string YMIR_TEST;
	    
	    static std::string YMIR_FUNCTION_RET;

	    static std::string YMIR_VAR;

	    static std::string YMIR_CST;

	    static std::string YMIR_VTABLE;

	    static std::string YMIR_TYPEINFO;

	private :

	    bool _fail;
	    
	private :

	    Mangler (bool fail);

	public :

	    /**
	     * \param fail, if fail is set to true, failure of mangling will make the compiler halt
	     */
	    static Mangler init (bool fail = true);

	    /**
	     * \return the prefix that will be put in the begining of mangled Ymir function
	     */
	    static std::string getYmirPrefix ();
	    
	    /**
	     * \brief Mangle the name of a given generator
	     * \param gen the generator
	     * \return the name of the generator
	     */
	    std::string mangle (const Generator & gen) const;

	    /**
	     * \brief Mangle a frame name
	     */
	    std::string mangleFrame (const Frame & fr) const;

	    /**
	     * \brief Mangle a frame name
	     */
	    std::string mangleTest (const Test & ts) const;

	    /**
	     * \brief Mangle a frame prototype
	     */
	    std::string mangleFrameProto (const FrameProto & fr) const;

	    /**
	     * \brief Mangle a constructor prototype
	     */
	    std::string mangleConstructorProto (const ConstructorProto & pr) const;
	    
	    /**
	     * \brief Mangle a global var name
	     */
	    std::string mangleGlobalVar (const GlobalVar & var) const;

	    /**
	     * \brief Mangle a global var constant
	     */
	    std::string mangleGlobalConstant (const GlobalConstant & cst) const;

	    /**
	     * \brief Mangle a type	     
	     */
	    std::string mangleType (const Generator & gen, bool fatherMutable) const;
	    
	    /**
	     * \brief Mangle a bool value
	     */
	    std::string mangleBoolV (const BoolValue & boo) const;

	    /**
	     * \brief Mangle a char value
	     */
	    std::string mangleCharV (const CharValue & ch) const;

	    /**
	     * \brief Mangle a string value
	     */
	    std::string mangleStringV (const StringValue & str) const;

	    /**
	     * \brief Mangle an integer value
	     */
	    std::string mangleFixed (const Fixed & fx) const;
	    
	    /**
	     * \brief Mangle a float value
	     */
	    std::string mangleFloatV (const FloatValue & fl) const;

	    /**
	     * \brief Mangle a tuple value
	     */
	    std::string mangleTupleV (const TupleValue & tp) const;

	    /**
	     * \brief Mangle a addr value (func ptr)
	     */
	    std::string mangleAddrV (const Addresser & addr) const;
	    
	    /**
	     * \brief Mangle an array type
	     */
	    std::string mangleArrayT (const Array & array) const;

	    /**
	     * \brief Mangle a bool type
	     */
	    std::string mangleBoolT (const Bool & boo) const ;

	    /**
	     * \brief Mangle a char type
	     */
	    std::string mangleCharT (const Char & ch) const ;

	    /**
	     * \brief Mangle a float type
	     */
	    std::string mangleFloatT (const Float & fl) const ;

	    /**
	     * \brief Mangle an integer type
	     */
	    std::string mangleIntegerT (const Integer & in) const ;

	    /**
	     * \brief Mangle a slice type
	     */
	    std::string mangleSliceT (const Slice & sl) const ;

	    /**
	     * \brief Mangle a tuple type
	     */
	    std::string mangleTupleT (const Tuple & tp) const ;

	    /**
	     * \brief Mangle the void type
	     */
	    std::string mangleVoidT (const Void & v) const ;

	    /**
	     * \brief Mangle a struct type
	     */
	    std::string mangleStructRef (const StructRef & r) const;
	    
	    /**
	     * \brief Mangle a class type
	     */
	    std::string mangleClassRef (const ClassRef & r) const;

	    /**
	     * \brief Mangle a class typeinfo name
	     */
	    std::string mangleTypeInfo (const ClassRef & r) const;
	    
	    /**
	     * \brief Mangle a class vtable name
	     */
	    std::string mangleVtable (const ClassRef & r) const;
	    
	    /**
	     * \brief Mangle a enum type
	     */
	    std::string mangleEnumRef (const EnumRef & r) const;

	    /**
	     * \brief Mangle a range type
	     */
	    std::string mangleRangeT (const Range & r) const;

	    /**
	     * \brief Mangle a pointer type
	     */
	    std::string manglePointerT (const Pointer & ptr) const;

	    /**
	     * \brief Mangle an option type
	     */
	    std::string mangleOptionT (const Option & o) const;
	    
	    /**
	     * \brief Mangle a class pointer type
	     */
	    std::string mangleClassPointerT (const ClassPtr & ptr) const;

	    /**
	     * \brief Mangle a function pointer type
	     */
	    std::string mangleFuncPtrT (const FuncPtr & ptr) const;

	    /**
	     * \brief Mangle a function pointer type
	     */
	    std::string mangleDelegateT (const Delegate & ptr) const;

	    /**
	     * \brief Mangle a closure type
	     */
	    std::string mangleClosureT (const Closure & c) const;

	    /**
	     * Mangle a string containing a path (delimited by '::')
	     */
	    std::string manglePath (const std::string & path) const;
	    
	private : 

	    /**
	     * \brief Split a string according to delimiter
	     */
	    std::vector <std::string> split (const std::string & str, const std::string & delim) const ;

	    
	};
	
    }    

}
