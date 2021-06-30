#include "generic.h"

#include <stdlib.h>

blob_t *new_blob() {
    blob_t *blob = malloc(sizeof(blob_t));

    return blob;
}
