#ifndef BLOCK_REPLACERS_H
#define BLOCK_REPLACERS_H

#include "blocks/matchers/matchers.h"
#include "blocks/block.h"
#include "blocks/stmt.h"
#include "blocks/expr.h"
#include "blocks/var.h"

namespace block {
namespace matcher {

void replace_match(block::Ptr, match, pattern::Ptr);

}
}
#endif
