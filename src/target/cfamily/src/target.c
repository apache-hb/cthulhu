#include "cfamily/target.h"

#include "cthulhu/broker/broker.h"
#include "target/target.h"

CT_TARGET_API const target_t kTargetC = {
    .info = {
        .id = "target-c",
        .name = "C",
        .version = {
            .license = "LGPLv3",
            .author = "Elliot Haisley",
            .desc = "C89 output target",
            .version = CT_NEW_VERSION(0, 0, 1)
        }
    }
};

CTU_TARGET_ENTRY(kTargetC)
