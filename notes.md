# notes
* useradd missing
* patch src/backend/utils/init/miscinit.c checkDataDir
* patch src/bin/initdb/initdb.c get_id
* patch src/backend/main/main.c check_root
* patch src/bin/pg_ctl/pg_ctl.c main
* patch src/bin/pg_upgrade/option.c parseCommandLine
* file permissions are broken
* file time isnt set on creation

# mysql
* patch include/volatile.h Vio
* implement my_rdtsc.cc
* add `#include <sys/stat.h>` to mf_tempfile.cc
* patch utilities/CMakeLists.txt to always build comp_err
* storage/ndb/src/common/portlib/NdbCondition.cpp add `#include <sys/time.h>`
* storage/ndb/src/common/portlib/NdbTick.cpp add `#include <sys/time.h>`
* storage/ndb/src/common/transporter/SHM_Transporter.hpp add `#include <sys/ipc.h>`
* router/src/harness/include/mysql/harness/net_ts/socket.h replace `__sun__` with `0`