#pragma once

#include <os_config.h>

#include "os/os.h"

#include <stdbool.h>

typedef struct os_mapping_t os_mapping_t;

CT_BEGIN_API

///
/// provided by the platform implementation
///

// get the last error
RET_INSPECT STA_LAST_ERROR
CT_LOCAL os_error_t impl_last_error(void);

// get the max length of a name or path
CT_LOCAL size_t impl_maxname(void);
CT_LOCAL size_t impl_maxpath(void);

// copies the file at src to dst, overwriting dst if it exists
// only needs to be implemented if CT_OS_COPYFILE is 1
CT_LOCAL os_error_t impl_copyfile(const char *dst, const char *src);

// files
CT_LOCAL os_file_impl_t impl_file_open(const char *path, os_access_t access);

// close a file handle
// return false if there was an error
CT_LOCAL bool impl_file_close(os_file_impl_t impl);

// file mapping
CT_LOCAL void *impl_file_map(os_file_t *file, os_protect_t protect, size_t size, os_mapping_t *map);
CT_LOCAL os_error_t impl_unmap(os_mapping_t *map);

// iteration

// returns true if there was a new inode
CT_LOCAL os_iter_impl_t impl_iter_open(const char *path, os_inode_impl_t *inode);
CT_LOCAL bool impl_iter_next(os_iter_impl_t impl, os_inode_impl_t *inode);
CT_LOCAL bool impl_iter_close(os_iter_impl_t impl);

CT_LOCAL const char *impl_inode_name(const os_inode_impl_t *inode);
CT_LOCAL os_dirent_t impl_inode_type(const os_inode_impl_t *inode);

// libraries
CT_LOCAL os_library_impl_t impl_library_open(const char *path);
CT_LOCAL bool impl_library_close(os_library_impl_t lib);
CT_LOCAL void *impl_library_symbol(os_library_impl_t lib, const char *name);

CT_END_API
