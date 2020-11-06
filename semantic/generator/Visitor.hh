#pragma once

#include <ymir/tree/_.hh>
#include <ymir/semantic/generator/_.hh>

namespace semantic {

    namespace generator {

	/**
	 * \struct Visitor
	 * This last visitor is the final visitor
	 * It will traverse all the generators produced by semantic analyses and translate them into gimple
	 * This visitor is called at lint time (lint meaning intermediate language)
	 * This part is the only part that is GCC dependent
	 */
	class Visitor {

	    /** The list of var decl for each blocks */
	    std::vector <generic::TreeChain> stackVarDeclChain;

	    /** The list of block currently entered */
	    std::vector <generic::BlockChain> stackBlockChain;	    	    

	    /** The global context of the generation */
	    generic::Tree _globalContext;

	    /** The global context of the generation */
	    generic::Tree _currentContext;

	    /** The declaration of the local var for each frame */
	    std::vector <std::map <uint, generic::Tree> > _declarators;

	    /** The declaration of the global var */
	    std::map <uint, generic::Tree> _globalDeclarators;

	    /**
	     * The value of global vars needing a static initialization
	     */
	    std::map <uint, std::pair <generator::Generator, generator::Generator> > _globalInitialiser;
	    
	    std::list <generic::Tree> _loopLabels;

	    std::list <generic::Tree> _loopVars;

	    /**
	     * List of defined frame, to prevent multiple definition of the same weak frame
	     */
	    std::set <std::string> _definedFrame;
	    
	private :

	    Visitor ();

	public :

	    /**
	     * Does nothing special
	     * It is private for homogeneity reason
	     * We wan't all class to be initialized the same way	     
	     */
	    static Visitor init () ;

	    /**
	     * Finalize the generation 
	     * Declare the remaining symbol into GCC memory
	     * (global declaration needs to be dones at the end ?)
	     */
	    void finalize ();	    
	    
	    /**
	     * \brief Generate gimple tree from a generator
	     * \brief Traverse all inner generator 
	     */
	    void generate (const Generator & gen);

	    /**
	     * \brief Generate a new symbol from a global var generator
	     * \param var the variable to generate
	     */
	    void generateGlobalVar (const GlobalVar & var);

	    /**
	     * \brief Init the frame, that will initialize all the global var before calling the main
	     */
	    void generateGlobalInitFrame ();
	    
	    /**
	     * \brief Generate the call of the _Ymain function from the runtime of Ymir
	     * \param isVoid is the main function declared as void?
	     * \param name the asmName of the function main
	     */
	    void generateMainCall (const lexing::Word & loc, bool isVoid, const std::string & name);
	    
	    /**
	     * \brief Generate a new frame from a frame generator
	     * \param frame the frame to generate
	     */
	    void generateFrame (const Frame & frame);

	    /**
	     * \brief Transform a global constant into gimple
	     */
	    generic::Tree generateGlobalConstant (const GlobalConstant & cst);

	    /**
	     * \brief Generate the type info for a class
	     * \brief This will be happened at the end of the vtable
	     */
	    generic::Tree generateTypeInfoClass (const Generator & classType);
	    
	    /**
	     * \brief Create the vtable of a classref
	     */
	    generic::Tree generateVtable (const Generator & classType);
	    
	    /**
	     * \brief Generate the declaration of a param var
	     * \return the declaration
	     */
	    generic::Tree  generateParamVar (const ParamVar & var);

	    /**
	     * \brief Generate a param var
	     * \Warning the declarator will not have any id, so it will not be inserted
	     * \Warning and therefore we cannot retreive it with a VarRef, yout will need to conserve the declaration if you want to use it 
	     * \param name the name of the parameter
	     * \param type the type of the parameter
	     * \return a tree containing a paramVar
	     */
	    generic::Tree generateParamVar (const std::string & name, const generic::Tree & type);
	    
	    /**
	     * \brief Transform a type from generator to gimple
	     */
	    generic::Tree generateType (const Generator & gen);

	    /**
	     * \brief Generate a class Type
	     */
	    generic::Tree generateClassType (const ClassRef & gen);
	    
	    /**
	     * \brief Generate the initial value of the type 
	     * \param type the type that will give a initial value
	     */
	    generic::Tree generateInitValueForType (const Generator & type);	    
	    
	    /**
	     * \brief Transform a value to gimple
	     * \param gen the generator of the value
	     */
	    generic::Tree generateValue (const Generator & gen);

	    /**
	     * \brief Transform a value to gimple
	     * \param type the type of the final expression
	     * \param gen the generator of the value
	     */
	    generic::Tree generateValue (const Generator & type, const Generator & gen);

	    /**
	     * \brief Transform a value to gimple
	     * \warning should be only called by generateValue
	     */
	    generic::Tree generateValueInner (const Generator & gen);
	    
	    /**
	     * \brief Transform a block into gimple
	     */
	    generic::Tree generateBlock (const Block & bl);

	    /**
	     * \brief Transform a left block into gimple
	     */
	    generic::Tree generateLeftBlock (const Block & bl);
	    
	    /**
	     * \brief Transform a block into gimple
	     */
	    generic::Tree generateSet (const Set & set);

	    /**
	     * \brief Transform a fixed value into gimple
	     */
	    generic::Tree generateFixed (const Fixed & fixed);

	    /**
	     * \brief Transform a fixed value into gimple
	     */
	    generic::Tree generateBool (const BoolValue & b);

	    /**
	     * \brief Transform a float value into gimple
	     */
	    generic::Tree generateFloat (const FloatValue & f);

	    /**
	     * \brief Transform a float value into gimple
	     */
	    generic::Tree generateChar (const CharValue & f);

	    /**
	     * \brief Transform a throw into gimple
	     */
	    generic::Tree generateThrow (const Throw & thr);

	    /**
	     * \brief Transform an exit scope into gimple
	     */
	    generic::Tree generateExitScope (const ExitScope & scope);

	    /**
	     * \brief Generate the catching part of an exit scope
	     * \param var the var containing the value of the scope
	     */
	    generic::Tree generateCatching (const ExitScope & scope, generic::Tree var);
	    
	    /**
	     * \brief Transform an success scope into gimple
	     */
	    generic::Tree generateSuccessScope (const SuccessScope & scope);	   
	    
	    /**
	     * \brief Transform an array value into gimple
	     */
	    generic::Tree generateArrayValue (const ArrayValue & arr);

	    /**
	     * \brief Transform a tuple value into gimple
	     */
	    generic::Tree generateTupleValue (const TupleValue & arr);

	    /**
	     * \brief Transform a string value into gimple
	     */
	    generic::Tree generateStringValue (const StringValue & str);
	    
	    /**
	     * \brief Transform a frame proto into gimple
	     */
	    generic::Tree generateFrameProto (const FrameProto & proto);

	    /**
	     * \brief Transform a constructor proto into gimple
	     */
	    generic::Tree generateConstructorProto (const ConstructorProto & proto);
	    
	    /**
	     * \brief Transform a tuple access into gimple
	     */
	    generic::Tree generateTupleAccess (const TupleAccess & acc);
	    
	    /**
	     * \brief Transform a struct access into gimple
	     */
	    generic::Tree generateStructAccess (const StructAccess & acc);

	    /**
	     * \brief Transform a vtable access into gimple
	     */	    
	    generic::Tree generateVtableAccess (const VtableAccess & acc);
	    
	    /**
	     * \brief Transform a struct construction into gimple
	     */
	    generic::Tree generateStructCst (const StructCst & cl);

	    /**
	     * \brief Transform an union construction into gimple
	     */
	    generic::Tree generateUnionCst (const UnionCst & cl);

	    /**
	     * \brief Transform a class construction into gimple
	     */
	    generic::Tree generateClassCst (const ClassCst & cl);
	    
	    /**
	     * \brief Transform a frame call into gimple
	     */
	    generic::Tree generateCall (const Call & cl);
	    
	    /**
	     * \brief Transform an affectation into gimple
	     */
	    generic::Tree generateAffect (const Affect & aff);
	    
	    /**
	     * \brief Transform a binary int generator into gimple
	     */
	    generic::Tree generateBinaryInt (const BinaryInt & bin);

	    /**
	     * \brief Transform a binary int generator into gimple
	     */
	    generic::Tree generateBinaryBool (const BinaryBool & bin);
	    
	    /**
	     * \brief Transform a binary int generator into gimple
	     */
	    generic::Tree generateBinaryFloat (const BinaryFloat & bin);

	    /**
	     * \brief Transform a binary ptr generator into gimple
	     */
	    generic::Tree generateBinaryPtr (const BinaryPtr & bin);
	    
	    /**
	     * \brief Transform a binary char generator into gimple
	     */
	    generic::Tree generateBinaryChar (const BinaryChar & bin);

	    /**
	     * \brief Transform a unary int generator into gimple
	     */
	    generic::Tree generateUnaryInt (const UnaryInt & un);
	    
	    /**
	     * \brief Transform a unary float generator into gimple
	     */
	    generic::Tree generateUnaryFloat (const UnaryFloat & un);

	    /**
	     * \brief Transform a unary bool generator into gimple
	     */
	    generic::Tree generateUnaryBool (const UnaryBool & un);

	    /**
	     * \brief Transform a unary pointer generator into gimple
	     */
	    generic::Tree generateUnaryPointer (const UnaryPointer & un);
	    
	    /**
	     * \brief Transform a var ref into gimple
	     */
	    generic::Tree generateVarRef (const VarRef & var);

	    /**
	     * \brief Transform a var decl into gimple
	     */
	    generic::Tree generateVarDecl (const VarDecl & var);

	    /**
	     * \brief Transform a referencer into gimple
	     */
	    generic::Tree generateReferencer (const Referencer & ref);

	    /**
	     * \brief Transform a addresser into gimple
	     */
	    generic::Tree generateAddresser (const Addresser & addr);

	    /**
	     * \brief Transform a conditional expression into gimple
	     */
	    generic::Tree generateConditional (const Conditional & cond);

	    /**
	     * \brief Transform a loop expression into gimple 
	     */
	    generic::Tree generateLoop (const Loop & loop);
	    
	    /**
	     * \brief Transform a break expression into gimple 
	     */
	    generic::Tree generateBreak (const Break & loop);

	    /**
	     * \brief Transform a return expression into gimple
	     */
	    generic::Tree generateReturn (const Return & ret);

	    /**
	     * \brief Transform a range value into gimple
	     */
	    generic::Tree generateRangeValue (const RangeValue & rg);
	    
	    /**
	     * \brief Transform a slice value into gimple
	     */
	    generic::Tree generateSliceValue (const SliceValue & rg);

	    /**
	     * \brief Transform a delegate value into gimple
	     */
	    generic::Tree generateDelegateValue (const DelegateValue & rg);

	    /**
	     * \brief Transform a cast value into gimple
	     */
	    generic::Tree generateCast (const Cast & cast);

	    /**
	     * \brief Generate an array allocation into gimple
	     */
	    generic::Tree generateArrayAlloc (const ArrayAlloc & alloc);

	    /**
	     * \brief Generate a null value
	     */
	    generic::Tree generateNullValue (const NullValue & nl);

	    /**
	     * \brief Generate an uniq value
	     */
	    generic::Tree generateUniqValue (const UniqValue & uv);
	    
	    /**
	     * \brief Transform a copier into a gimple tree
	     */
	    generic::Tree generateCopier (const Copier & copy);

	    /**
	     * \brief Transform a sizeof into a gimple tree
	     */
	    generic::Tree generateSizeOf (const SizeOf & copy);
	    
	    /**
	     * \brief Transform a copier into a gimple tree
	     */
	    generic::Tree generateAliaser (const Aliaser & als);

	    /**
	     * \brief Transform an array access into gimple
	     */
	    generic::Tree generateArrayAccess (const ArrayAccess & access);

	    /**
	     * \brief Transform an slice access into gimple
	     */
	    generic::Tree generateSliceAccess (const SliceAccess & access);

	    /**
	     * \brief Transform a slice concatenation into gimple
	     */
	    generic::Tree generateSliceConcat (const SliceConcat & concat);
	    
	private :

	    /**
	     * Enter a new block 
	     */
	    void enterBlock () ;

	    /** 
	     * Quit the last block and return its symbol mapping 
	    */
	    generic::TreeSymbolMapping quitBlock (const lexing::Word & loc, const generic::Tree &);

	    /**
	     * \brief Enter a new loop
	     * \param endLabel the label to jump in case of a break
	     * \param var where to store the value in case of a break
	     */
	    void enterLoop (const generic::Tree & endLabel, const generic::Tree & var);

	    /**
	     * \brief Quit a loop (normally)
	     */
	    void quitLoop ();
	    
	    /**
	     * Enter a new Frame
	     */
	    void enterFrame ();

	    /**
	     * Close the lastFrame 
	     */
	    void quitFrame ();

	    /**
	     * \return the tree reprensenting the global context of definition
	     */
	    const generic::Tree & getGlobalContext ();
	    
	    /**
	     * \return the tree reprensenting the current context of definition
	     */
	    const generic::Tree & getCurrentContext () const;

	    /**
	     * \brief Change the current context for definition
	     */
	    void setCurrentContext (const generic::Tree & tr);

	    /**
	     * \brief Add a new vardecl for future var referencing 
	     */
	    void insertDeclarator (uint id, const generic::Tree & decl);

	    /**
	     * \brief Add a new vardecl for future var ref in the global context
	     */
	    void insertGlobalDeclarator (uint id, const generic::Tree & decl);
	    
	    /**
	     * \brief Get a var declarator
	     * \warning make the compiler crash if the id does not exist
	     */
	    generic::Tree getDeclarator (uint id);
	    	    
	    /**
	     * \brief Get a var declarator or empty if not exist
	     */
	    generic::Tree getDeclaratorOrEmpty (uint id);
	    
	    /**
	     * \brief transform the value of value into type
	     */
	    generic::Tree castTo (const Generator & type, const Generator & value);

	    /**
	     * \brief Identifiy the type of the gen
	     * \return its name
	     */
	    std::string identify (const Generator & gen);

	};

    }
    
}
