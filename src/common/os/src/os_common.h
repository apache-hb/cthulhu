#pragma once

#include <os_config.h>

#include "os/core.h"

CT_BEGIN_API

///
/// provided by os_common
///

///
/// provided by the platform implementation
///

// get the last error
CT_LOCAL os_error_t impl_last_error(void);

// get the name of an inode
CT_LOCAL const char *impl_dirname(const os_inode_t *inode);

// get the max length of a name or path
CT_LOCAL size_t impl_maxname(void);
CT_LOCAL size_t impl_maxpath(void);

// copies the file at src to dst, overwriting dst if it exists
// only needs to be implemented if CT_OS_COPYFILE is 1
CT_LOCAL os_error_t impl_copyfile(const char *dst, const char *src);

CT_END_API
