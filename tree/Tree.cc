#include <ymir/tree/Tree.hh>

namespace generic {

    Tree::Tree () :
	_t (NULL_TREE),
	_loc (UNKNOWN_LOCATION)
    {}

    Tree Tree::init (const location_t & loc, const tree & t) {
	auto ret = Tree ();
	ret._t = t;
	ret._loc = loc;
	return ret;
    }

    Tree Tree::error () {
	auto ret = Tree ();
	ret._t = error_mark_node;
	ret._loc = UNKNOWN_LOCATION;
	return ret;
    }

    Tree Tree::empty () {
	return Tree ();
    }

    
    Tree Tree::block (const lexing::Word & loc, const Tree & vars, const Tree & chain) {
	return Tree::init (
	    loc.getLocus (), build_block (
		vars.getTree (), chain.getTree (), NULL_TREE, NULL_TREE
	    )
	);
    }
    
    Tree Tree::voidType () {
	return Tree::init (UNKNOWN_LOCATION, void_type_node);
    }
    
    Tree Tree::varDecl (const lexing::Word & loc, const std::string & name, const Tree & type) {
	return Tree::init (loc.getLocus (),
			   build_decl (
			       loc.getLocus (),
			       VAR_DECL,
			       get_identifier (name.c_str ()),
			       type.getTree ()
			   )
	);
    }
    
    Tree Tree::paramDecl (const lexing::Word & loc, const std::string & name, const Tree & type) {
	return Tree::init (loc.getLocus (),
			   build_decl (
			       loc.getLocus (),
			       PARM_DECL,
			       get_identifier (name.c_str ()),
			       type.getTree ()
			   )
	);
    }

    Tree Tree::functionType (const Tree & retType, const std::vector <Tree> & params) {
	std::vector <tree> tree_params (params.size ());
	for (auto i : Ymir::r (0, params.size ()))
	    tree_params [i] = params [i].getTree ();
	
	return Tree::init (UNKNOWN_LOCATION,
			   build_function_type_array (
			       retType.getTree (),
			       tree_params.size (),
			       tree_params.data ()
			   )
	);
    }

    Tree Tree::functionDecl (const lexing::Word & loc, const std::string & name, const Tree & type) {
	return Tree::init (loc.getLocus (),
			   build_decl (
			       loc.getLocus (),
			       FUNCTION_DECL,
			       get_identifier (name.c_str ()),
			       type.getTree ()
			   )
	);			   
    }
    
    Tree Tree::resultDecl (const lexing::Word & loc, const Tree & type) {
	return Tree::init (
	    loc.getLocus (),
	    build_decl (
		loc.getLocus (),
		RESULT_DECL,
		NULL_TREE,
		type.getTree ()
	    )
	);			
    }    

    Tree Tree::build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1) {
	return Tree::init (loc.getLocus (), build1_loc (loc.getLocus (), tc, type.getTree (), t1.getTree ()));
    }
    
    Tree Tree::build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2) {
	return Tree::init (loc.getLocus (), build2_loc (loc.getLocus (), tc, type.getTree (), t1.getTree (), t2.getTree ()));
    }

    Tree Tree::build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3) {
	return Tree::init (loc.getLocus (), build3_loc (loc.getLocus (), tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree ()));
    }

    Tree Tree::build (tree_code tc, const lexing::Word & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4) {
	return Tree::init (loc.getLocus (), build4_loc (loc.getLocus (), tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree (), t4.getTree ()));
    }

    Tree Tree::build (tree_code tc, lexing::Word loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4, const Tree & t5) {
	return Tree::init (loc.getLocus (), build5_loc (loc.getLocus (), tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree (), t4.getTree (), t5.getTree ()));
    }
    
    const location_t & Tree::getLocus () const {
	return this-> _loc;
    }

    void Tree::setLocus (const location_t & loc) {
	this-> _loc = loc;
    }

    const tree & Tree::getTree () const {
	return this-> _t;
    }

    void Tree::setTree (const tree & t) {
	this-> _t = t;
    }

    tree_code Tree::getTreeCode () const {
	return TREE_CODE (this-> _t);
    }

    bool Tree::isError () const {
	return error_operand_p (this-> _t);
    }

    bool Tree::isEmpty () const {
	return this-> _t == NULL_TREE;
    }

    bool Tree::isStatic () const {
	return TREE_STATIC (this-> _t) == 1;
    }
    
    void Tree::isStatic (bool st) {
	TREE_STATIC (this-> _t) = st;
    }

    bool Tree::isUsed () const {
	return TREE_USED (this-> _t) == 1;
    }
    
    void Tree::isUsed (bool us) {
	TREE_USED (this-> _t) = us;
    }

    bool Tree::isExternal () const {
	return DECL_EXTERNAL (this-> _t) == 1;
    }
    
    void Tree::isExternal (bool ext) {
	DECL_EXTERNAL (this-> _t) = ext;
    }

    bool Tree::isPreserved () const {
	return DECL_PRESERVE_P (this-> _t) == 1;
    }
    
    void Tree::isPreserved (bool pre) {
	DECL_PRESERVE_P (this-> _t) = pre;
    }

    bool Tree::isPublic () const {
	return TREE_PUBLIC (this-> _t) == 1;
    }
    
    void Tree::isPublic (bool b) {
	TREE_PUBLIC (this-> _t) = b;
    }

    Tree Tree::getDeclContext () const {
	return Tree::init (
	    UNKNOWN_LOCATION,
	    DECL_CONTEXT (this-> _t)
	);			   
    }
    
    void Tree::setDeclContext (const Tree & context) {
	DECL_CONTEXT (this-> _t) = context.getTree ();
    }

    Tree Tree::getDeclInitial () const {
	return Tree::init (
	    UNKNOWN_LOCATION,
	    DECL_INITIAL (this-> _t)
	);			   	
    }

    void Tree::setDeclInitial (const Tree & init) {
	DECL_INITIAL (this-> _t) = init.getTree ();
    }

    Tree Tree::getDeclSavedTree () const {
	return Tree::init (
	    UNKNOWN_LOCATION,
	    DECL_SAVED_TREE (this-> _t)
	);			   	
    }

    void Tree::setDeclSavedTree (const Tree & saved) {
	DECL_SAVED_TREE (this-> _t) = saved.getTree ();
    }

    bool Tree::isWeak () const {
	return DECL_WEAK (this-> _t) == 1;
    }

    void Tree::isWeak (bool is) {
	DECL_WEAK (this-> _t) = is;
    }

    void Tree::setDeclArguments (const std::list <Tree> & args) {
	tree arglist = NULL_TREE; 
	for (auto & it : args)
	    arglist = chainon (arglist, it.getTree ());
	DECL_ARGUMENTS (this-> _t) = arglist;
    }

    void Tree::setArgType (const Tree & type) {
	DECL_ARG_TYPE (this-> _t) = type.getTree ();
    }

    void Tree::setResultDecl (const Tree & result) {
	DECL_RESULT (this-> _t) = result.getTree ();
    }

    void Tree::setBlockSuperContext (const Tree & tree) {
	BLOCK_SUPERCONTEXT (this-> _t) = tree.getTree ();
    }
    
    Tree Tree::getType () const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return Tree::init (this-> _loc, TREE_TYPE (this-> _t));
    }

    Tree Tree::getOperand (int i) const {
	if (this-> _t == NULL_TREE)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return Tree::init (this-> _loc, TREE_OPERAND (this-> _t, i));
    }

    
}
