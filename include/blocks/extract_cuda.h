#ifndef BLOCKS_EXTRACT_CUDA_H
#define BLOCKS_EXTRACT_CUDA_H

#include "blocks/annotation_finder.h"
#include "blocks/block.h"
#include "blocks/block_visitor.h"
#include "blocks/expr.h"
#include "blocks/stmt.h"
#include <algorithm>
#include <iostream>
#include <vector>

#define CUDA_KERNEL "kernel:cuda:auto"
#define CUDA_KERNEL_COOP "kernel:cuda:coop"

namespace block {

std::vector<block::Ptr> extract_cuda_from(block::Ptr from);

block::Ptr extract_single_cuda(block::Ptr from, std::vector<decl_stmt::Ptr> &);
extern int total_created_kernels;

class gather_extern_vars : public block_visitor {
public:
	using block_visitor::visit;
	std::vector<var::Ptr> gathered;
	std::vector<var::Ptr> declared;
	virtual void visit(var_expr::Ptr);
};

class cuda_var_replacer : public block_visitor {
public:
	using block_visitor::visit;
	var::Ptr to_replace;
	var::Ptr replace_with;

	virtual void visit(var_expr::Ptr);
};

class gather_declared_vars : public block_visitor {
public:
	using block_visitor::visit;
	std::vector<var::Ptr> declared;
	virtual void visit(decl_stmt::Ptr);
	virtual void visit(func_decl::Ptr);
};

} // namespace block
#endif
