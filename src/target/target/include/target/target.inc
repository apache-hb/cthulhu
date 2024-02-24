// SPDX-License-Identifier: GPL-3.0-only

#ifndef FILE_LAYOUT
#   define FILE_LAYOUT(id, str)
#endif

FILE_LAYOUT(eFileLayoutTree, "tree")
FILE_LAYOUT(eFileLayoutFlat, "flat")
FILE_LAYOUT(eFileLayoutSingle, "single")
FILE_LAYOUT(eFileLayoutPair, "pair")

#undef FILE_LAYOUT
