#pragma once


#include "tree.h"



/* The context to be used for global declarations.  */
extern tree __global_context__;

/* Array of all global declarations to pass back to the middle-end.  */
extern vec <tree, va_gc> *__global_declarations__;

extern tree __current_function_ctx__;

tree ymir_get_global_context ();

enum y_tree_index
{
  YTI_BOOL_TYPE,
  YTI_C8_TYPE,
  YTI_C16_TYPE,
  YTI_C32_TYPE,
  YTI_I8_TYPE,
  YTI_U8_TYPE,
  YTI_I16_TYPE,
  YTI_U16_TYPE,
  YTI_I32_TYPE,
  YTI_U32_TYPE,
  YTI_I64_TYPE,
  YTI_U64_TYPE,
  YTI_ISIZE_TYPE,
  YTI_USIZE_TYPE,
  YTI_F32_TYPE,
  YTI_F64_TYPE,
  YTI_MAX
};


extern GTY(()) tree y_global_trees[YTI_MAX];

#define y_bool_type			y_global_trees[YTI_BOOL_TYPE]
#define y_i8_type			y_global_trees[YTI_I8_TYPE]
#define y_u8_type			y_global_trees[YTI_U8_TYPE]
#define y_i16_type			y_global_trees[YTI_I16_TYPE]
#define y_u16_type			y_global_trees[YTI_U16_TYPE]
#define y_i32_type			y_global_trees[YTI_I32_TYPE]
#define y_u32_type			y_global_trees[YTI_U32_TYPE]
#define y_i64_type			y_global_trees[YTI_I64_TYPE]
#define y_u64_type			y_global_trees[YTI_U64_TYPE]
#define y_isize_type			y_global_trees[YTI_ISIZE_TYPE]
#define y_usize_type			y_global_trees[YTI_USIZE_TYPE]
#define y_f32_type			y_global_trees[YTI_F32_TYPE]
#define y_f64_type			y_global_trees[YTI_F64_TYPE]
#define y_c8_type			y_global_trees[YTI_C8_TYPE]
#define y_c16_type			y_global_trees[YTI_C16_TYPE]
#define y_c32_type			y_global_trees[YTI_C32_TYPE]


extern "C" void ymir_binding_parse_file (uint32_t nb_files, const char ** file);
