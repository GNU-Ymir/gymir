#pragma once

#include "Expression.hh"
#include "../semantic/types/InfoType.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IArrayInfo;
    typedef IArrayInfo* ArrayInfo;
}

namespace syntax {

    /**
     * \struct IArrayAlloc
     * The syntaxic node representation of an array allocation 
     * \verbatim
     array_alloc := '[' type ';' ('new' expression | expression) ']'
     \endverbatim
     */
    class IArrayAlloc : public IExpression {

	Expression _type;
	
	Expression _size;
	
	semantic::InfoType _cster;

	bool _isImmutable;
	
    public:

	/**
	 * \param token the location of the array allocation
	 * \param type the type of the allocation
	 * \param size the size of the allocation
	 * \param isImmutable is the allocation is static or dynamic ? (true if static)
	 */
	IArrayAlloc (Word token, Expression type, Expression size, bool isImmutable = false);
	
	Expression expression () override;

	/**
	 * \brief Create a type based on an allocation (inside template for example) 
	 * \verbatim
	 foo!([i32 ; 10]); // [i32 ; 10] is an ArrayAlloc, but is used as a type
	 \endverbatim
	 */
	Expression staticArray ();
	
	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Expression onClone () override;

	/**
	 * \return the type of the allocation
	 */
	Expression getType ();

	/**
	 * \return the size of the allocation
	 */
	Expression getSize ();
	
	Ymir::Tree toGeneric () override;

	static const char * id () {
	    return TYPEID (IArrayAlloc);
	}
	
	virtual std::vector <std::string> getIds ();
	
	std::string prettyPrint () override;

	void print (int nb = 0) override;

	virtual ~IArrayAlloc ();
	
    private :

	/**
	 * \brief Generate the code of the creation of a static array
	 * \param info the type of the array to generate
	 * \param inner the type of inner elements
	 * \param array the type of the array (in generic GCC internal)
	 */
	Ymir::Tree staticGeneric (semantic::ArrayInfo info, Ymir::Tree inner, Ymir::Tree array);

	/**
	 * \brief Generate the code of the creation of a dynamic array
	 * \param info the type of the array to generate
	 * \param inner the type of inner elements
	 * \param array the type of the array (in generic GCC internal)
	 */
	Ymir::Tree dynamicGeneric (semantic::ArrayInfo info, Ymir::Tree inner, Ymir::Tree array);
	
    };

    typedef IArrayAlloc* ArrayAlloc;
}
