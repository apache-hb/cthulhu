#ifndef OS_ACCESS
#   define OS_ACCESS(id, str, value)
#endif

OS_ACCESS(eOsAccessRead, "read", (1 << 0)) ///< file is readable
OS_ACCESS(eOsAccessWrite, "write", (1 << 1)) ///< file is writeable, does not imply readable
OS_ACCESS(eOsAccessTruncate, "truncate", (1 << 2)) ///< file is truncated if it exists

#undef OS_ACCESS

#ifndef OS_PROTECT
#   define OS_PROTECT(id, str, value)
#endif

OS_PROTECT(eOsProtectNone, "none", 0) ///< memory cannot be accessed
OS_PROTECT(eOsProtectRead, "read", (1 << 0)) ///< memory can be read
OS_PROTECT(eOsProtectWrite, "write", (1 << 1)) ///< memory can be written to (does not imply read)
OS_PROTECT(eOsProtectExecute, "execute", (1 << 2)) ///< memory can be executed (does not imply read or write)

#undef OS_PROTECT

#ifndef OS_DIRENT
#   define OS_DIRENT(id, str)
#endif

OS_DIRENT(eOsNodeNone, "unknown")
OS_DIRENT(eOsNodeFile, "file")
OS_DIRENT(eOsNodeDir, "directory")
OS_DIRENT(eOsNodeError, "error")

#undef OS_DIRENT
