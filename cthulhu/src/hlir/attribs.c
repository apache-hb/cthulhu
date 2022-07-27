#include "cthulhu/hlir/attribs.h"
#include "base/memory.h"

hlir_attributes_t *hlir_attributes(hlir_linkage_t linkage, hlir_visibility_t visibility, hlir_tags_t tags, const char *name)
{
    hlir_attributes_t *attributes = ctu_malloc(sizeof(hlir_attributes_t));
    attributes->linkage = linkage;
    attributes->visibility = visibility;
    attributes->tags = tags;
    attributes->mangle = name;
    return attributes;
}

hlir_attributes_t *hlir_linkage(hlir_linkage_t linkage)
{
    return hlir_attributes(linkage, DEFAULT_VISIBILITY, DEFAULT_TAGS, NULL);
}

hlir_attributes_t *hlir_tags(hlir_tags_t tags)
{
    return hlir_attributes(DEFAULT_LINKAGE, DEFAULT_VISIBILITY, tags, NULL);
}

bool is_entry_point(hlir_linkage_t linkage)
{
    return linkage == eLinkEntryCli || linkage == eLinkEntryGui;
}
