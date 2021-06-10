#include "report/report.h"
#include "front/compile.h"

int main() {
    compile_string("string", "5 + 99999999999999999999999999999999999996;");
    
    uint64_t errs = errors();

    if (errs) {
        reportf("%llu total compile errors", errs);
        return 1;
    }
    
    return 0;
}
