#include "cthulhu/hlir/attribs.h"
#include "base/memory.h"

hlir_attributes_t *hlir_attributes(linkage_t linkage, visibility_t visibility, tags_t tags, const char *name)
{
    hlir_attributes_t *attributes = ctu_malloc(sizeof(hlir_attributes_t));
    attributes->linkage = linkage;
    attributes->visibility = visibility;
    attributes->tags = tags;
    attributes->mangle = name;
    return attributes;
}

hlir_attributes_t *hlir_linkage(linkage_t linkage)
{
    return hlir_attributes(linkage, DEFAULT_VISIBILITY, DEFAULT_TAGS, NULL);
}

hlir_attributes_t *hlir_tags(tags_t tags)
{
    return hlir_attributes(DEFAULT_LINKAGE, DEFAULT_VISIBILITY, tags, NULL);
}

bool is_entry_point(linkage_t linkage)
{
    return linkage == eLinkEntryCli || linkage == eLinkEntryGui;
}
