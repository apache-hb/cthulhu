#include "common.h"
#include "cthulhu/loader/loader.h"
#include "cthulhu/util/file.h"
#include "cthulhu/util/str.h"

#include <string.h>

static bool read_bytes(data_t *data, void *dst, size_t *cursor, size_t size)
{
    if (data->length < *cursor + size)
    {
        report(data->header.reports, ERROR, NULL, "out of bounds read at %zu", *cursor);
        return false;
    }

    memcpy(dst, data->data + *cursor, size);
    *cursor += size;

    return true;
}

static char *read_string(data_t *in, size_t *cursor)
{
    offset_t offset;
    bool ok = read_bytes(in, &offset, cursor, sizeof(offset_t));
    if (!ok || offset == NULL_OFFSET)
    {
        return NULL;
    }

    return ctu_strdup(in->data + in->string + offset);
}

static value_t read_value(data_t *in, field_t field, size_t *offset)
{
    char *tmp;
    value_t value;
    bool ok = true;

    switch (field)
    {
    case FIELD_STRING:
        value.string = read_string(in, offset);
        break;
    case FIELD_INT:
        tmp = read_string(in, offset);
        mpz_init_set_str(value.digit, tmp, DIGIT_BASE);
        break;
    case FIELD_BOOL:
        ok = read_bytes(in, &value.boolean, offset, sizeof(bool_t));
        break;
    case FIELD_REFERENCE:
        ok = read_bytes(in, &value.reference, offset, sizeof(index_t));
        break;
    case FIELD_ARRAY:
        ok = read_bytes(in, &value.array, offset, sizeof(array_t));
        break;

    default:
        report(in->header.reports, ERROR, NULL, "invalid field type %d", field);
        ok = false;
        break;
    }

    if (!ok)
    {
        report(in->header.reports, ERROR, NULL, "failed to read value");
        return reference_value(NULL_INDEX);
    }

    return value;
}

static const char *compatible_version(uint32_t file, uint32_t expected)
{
    if (VERSION_MAJOR(expected) != VERSION_MAJOR(file))
    {
        return "major";
    }

    if (VERSION_MINOR(expected) != VERSION_MINOR(file))
    {
        return "minor";
    }

    return NULL;
}

bool is_loadable(const char *path, uint32_t submagic, uint32_t version)
{
    cerror_t error = 0;
    file_t handle = file_open(path, eFileBinary, &error);

    if (error != 0)
    {
        return false;
    }

    basic_header_t basic;
    size_t read = file_read(handle, &basic, sizeof(basic), &error);

    if (error != 0)
    {
        return false;
    }

    if (read < sizeof(basic_header_t))
    {
        return false;
    }

    if (basic.magic != FILE_MAGIC)
    {
        return false;
    }

    if (basic.submagic != submagic)
    {
        return false;
    }

    if (compatible_version(basic.semver, version) != NULL)
    {
        return false;
    }

    file_close(handle);

    return true;
}

#define NUM_TYPES(header) ((header).format->types)

bool begin_load(data_t *in, header_t header)
{
    begin_data(in, header);
    const char *path = header.path;

    cerror_t error = 0;

    file_t file = file_open(path, eFileBinary, &error);
    if (error != 0)
    {
        return false;
    }

    in->data = file_map(file, &error);
    in->length = file_size(file, &error);

    offset_t cursor = 0;
    size_t len = NUM_TYPES(header);

    basic_header_t basic;
    bool ok = read_bytes(in, &basic, &cursor, sizeof(basic_header_t));
    if (!ok)
    {
        report(header.reports, ERROR, NULL, "failed to read basic header");
        return false;
    }

    in->string = basic.strings;
    in->array = basic.arrays;

    if (basic.magic != FILE_MAGIC)
    {
        report(header.reports, ERROR, NULL, "[%s] invalid magic number. found %x, expected %x", path, basic.magic,
               FILE_MAGIC);
        return false;
    }

    if (basic.submagic != header.submagic)
    {
        report(header.reports, ERROR, NULL, "[%s] invalid submagic number. found %x, expected %x", path, basic.submagic,
               header.submagic);
        return false;
    }

    const char *err = compatible_version(basic.semver, header.semver);
    if (err != NULL)
    {
        report(header.reports, ERROR, NULL, "[%s] incompatible version. found %u.%u.%u, expected %u.%u.%u", path,
               VERSION_MAJOR(basic.semver), VERSION_MINOR(basic.semver), VERSION_PATCH(basic.semver),
               VERSION_MAJOR(header.semver), VERSION_MINOR(header.semver), VERSION_PATCH(header.semver));
        return false;
    }

    offset_t *counts = ctu_malloc(sizeof(offset_t) * len);
    offset_t *offsets = ctu_malloc(sizeof(offset_t) * len);

    ok = read_bytes(in, counts, &cursor, sizeof(offset_t) * len);
    if (!ok)
    {
        report(header.reports, ERROR, NULL, "failed to read count array");
        return false;
    }

    ok = read_bytes(in, offsets, &cursor, sizeof(offset_t) * len);
    if (!ok)
    {
        report(header.reports, ERROR, NULL, "failed to read offset array");
        return false;
    }

    in->counts = counts;
    in->offsets = offsets;

    return true;
}

void end_load(data_t *in)
{
    end_data(in);
}

bool read_entry(data_t *in, index_t index, value_t *values)
{
    size_t type = index.type;

    if (index.type == NULL_TYPE || index.offset == NULL_OFFSET)
    {
        report(in->header.reports, ERROR, NULL, "null read attempted");
        return false;
    }

    if (type > NUM_TYPES(in->header))
    {
        report(in->header.reports, ERROR, NULL, "invalid type index 0x%zx", type);
        return false;
    }

    size_t scale = in->sizes[type] * index.offset;
    size_t offset = in->offsets[type] + scale + sizeof(basic_header_t);

    if (offset > in->length)
    {
        report(in->header.reports, ERROR, NULL, "invalid offset %zu", offset);
        return false;
    }

    layout_t layout = in->header.format->layouts[type];
    size_t len = layout.length;

    for (size_t i = 0; i < len; i++)
    {
        values[i] = read_value(in, layout.fields[i], &offset);
    }

    return true;
}

bool read_array(data_t *in, array_t array, index_t *indices)
{
    memcpy(indices, in->data + in->array + array.offset, sizeof(index_t) * array.length);
    return true;
}
