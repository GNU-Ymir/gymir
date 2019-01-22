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

    Tree Tree::block (const location_t & loc, const Tree & vars, const Tree & chain) {
	return Tree::init (
	    loc, build_block (
		vars.getTree (), chain.getTree (), NULL_TREE, NULL_TREE
	    )
	);
    }
    
    Tree Tree::build (tree_code tc, const location_t & loc, const Tree & type, const Tree & t1) {
	return Tree::init (loc, build1_loc (loc, tc, type.getTree (), t1.getTree ()));
    }
    
    Tree Tree::build (tree_code tc, const location_t & loc, const Tree & type, const Tree & t1, const Tree & t2) {
	return Tree::init (loc, build2_loc (loc, tc, type.getTree (), t1.getTree (), t2.getTree ()));
    }

    Tree Tree::build (tree_code tc, const location_t & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3) {
	return Tree::init (loc, build3_loc (loc, tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree ()));
    }

    Tree Tree::build (tree_code tc, const location_t & loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4) {
	return Tree::init (loc, build4_loc (loc, tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree (), t4.getTree ()));
    }

    Tree Tree::build (tree_code tc, location_t loc, const Tree & type, const Tree & t1, const Tree & t2, const Tree & t3, const Tree & t4, const Tree & t5) {
	return Tree::init (loc, build5_loc (loc, tc, type.getTree (), t1.getTree (), t2.getTree (), t3.getTree (), t4.getTree (), t5.getTree ()));
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

    void Tree::isStatic (bool st) {
	TREE_STATIC (this-> _t) = st;
    }

    void Tree::isUsed (bool us) {
	TREE_USED (this-> _t) = us;
    }

    void Tree::isExternal (bool ext) {
	DECL_EXTERNAL (this-> _t) = ext;
    }

    void Tree::preservePointer (bool pre) {
	DECL_PRESERVE_P (this-> _t) = pre;
    }

    void Tree::isPublic (bool b) {
	TREE_PUBLIC (this-> _t) = b;
    }

    void Tree::setDeclContext (const Tree & context) {
	DECL_CONTEXT (this-> _t) = context.getTree ();
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
