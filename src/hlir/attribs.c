#include "cthulhu/hlir/attribs.h"
#include "cthulhu/util/util.h"

hlir_attributes_t *hlir_attributes(hlir_linkage_t linkage, hlir_tags_t tags, const char *name, const char *module)
{
    hlir_attributes_t *attributes = ctu_malloc(sizeof(hlir_attributes_t));
    attributes->linkage = linkage;
    attributes->tags = tags;
    attributes->mangle = name;
    attributes->module = module;
    return attributes;
}

hlir_attributes_t *hlir_linkage(hlir_linkage_t linkage)
{
    return hlir_attributes(linkage, DEFAULT_TAGS, NULL, NULL);
}

hlir_attributes_t *hlir_tags(hlir_tags_t tags)
{
    return hlir_attributes(DEFAULT_LINKAGE, tags, NULL, NULL);
}
