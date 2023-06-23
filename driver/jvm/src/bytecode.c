#include "bytecode.h"
#include "jvm.h"

#include "cthulhu/mediator/driver.h"

#include "base/endian.h"
#include "base/panic.h"
#include "base/memory.h"

#include "scan/scan.h"

#include "report/report.h"

typedef struct jvm_class_t {
    uint16_t minorVersion;
    jvm_version_t majorVersion;
} jvm_class_t;

#define FN_READ(name, type, err) \
    static type name(scan_t *scan) \
    { \
        type value = (err); \
        CTASSERT(scan_read(scan, &value, sizeof(type)) == sizeof(type)); \
        return value; \
    }

FN_READ(read8, uint8_t, UINT8_MAX)
FN_READ(read16, uint16_t, UINT16_MAX)
FN_READ(read32, uint32_t, UINT32_MAX)

static uint16_t read_be16(scan_t *scan)
{
    return native_order16(read16(scan), eEndianBig);
}

static uint32_t read_be32(scan_t *scan)
{
    return native_order32(read32(scan), eEndianBig);
}

static jvm_utf8_info_t utf8_info_read(scan_t *scan)
{
    uint16_t length = read_be16(scan);
    uint8_t *bytes = ctu_malloc(length + 1);
    CTASSERT(scan_read(scan, bytes, length) == length);
    
    bytes[length] = '\0';

    logverbose("utf8(length=%u, bytes=`%s`)", length, bytes);

    jvm_utf8_info_t value = {
        .length = length,
        .bytes = bytes
    };

    return value;
}

static jvm_class_info_t class_info_read(scan_t *scan)
{
    uint16_t nameIndex = read_be16(scan);

    logverbose("class(nameIndex=%u)", nameIndex);

    jvm_class_info_t value = {
        .nameIndex = nameIndex
    };

    return value;
}

static jvm_field_info_t field_info_read(scan_t *scan)
{
    uint16_t classIndex = read_be16(scan);
    uint16_t nameAndTypeIndex = read_be16(scan);

    logverbose("field(classIndex=%u, nameAndTypeIndex=%u)", classIndex, nameAndTypeIndex);

    jvm_field_info_t value = {
        .classIndex = classIndex,
        .nameAndTypeIndex = nameAndTypeIndex
    };

    return value;
}

static jvm_method_type_info_t method_type_info_read(scan_t *scan)
{
    uint16_t descriptorIndex = read_be16(scan);

    logverbose("methodType(descriptorIndex=%u)", descriptorIndex);

    jvm_method_type_info_t value = {
        .descriptorIndex = descriptorIndex
    };

    return value;
}

static jvm_name_and_type_info_t name_and_type_info_read(scan_t *scan)
{
    uint16_t nameIndex = read_be16(scan);
    uint16_t descriptorIndex = read_be16(scan);

    logverbose("nameAndType(nameIndex=%u, descriptorIndex=%u)", nameIndex, descriptorIndex);

    jvm_name_and_type_info_t value = {
        .nameIndex = nameIndex,
        .descriptorIndex = descriptorIndex
    };

    return value;
}

static jvm_method_handle_info_t method_handle_info_read(scan_t *scan)
{
    uint8_t referenceKind = read8(scan);
    uint16_t referenceIndex = read_be16(scan);

    logverbose("methodHandle(referenceKind=%u, referenceIndex=%u)", referenceKind, referenceIndex);

    jvm_method_handle_info_t value = {
        .referenceKind = referenceKind,
        .referenceIndex = referenceIndex
    };

    return value;
}

static jvm_invoke_dynamic_info_t invoke_dynamic_info_read(scan_t *scan)
{
    uint16_t bootstrapMethodAttrIndex = read_be16(scan);
    uint16_t nameAndTypeIndex = read_be16(scan);

    logverbose("invokeDynamic(bootstrapMethodAttrIndex=%u, nameAndTypeIndex=%u)", bootstrapMethodAttrIndex, nameAndTypeIndex);

    jvm_invoke_dynamic_info_t value = {
        .bootstrapMethodAttrIndex = bootstrapMethodAttrIndex,
        .nameAndTypeIndex = nameAndTypeIndex
    };

    return value;
}

static jvm_float_info_t float_info_read(scan_t *scan)
{
    uint32_t bytes = read_be32(scan);
    float value = *(float *)&bytes;

    // TODO: check this
    logverbose("float(bytes=%u, float=%f)", bytes, value);

    jvm_float_info_t it = {
        .value = bytes
    };

    return it;
}

static jvm_const_t const_read(scan_t *scan)
{
    uint8_t tag = read8(scan);

    logverbose("const(tag=%s)", jvm_const_tag_string(tag));

    jvm_const_t value = {
        .tag = tag
    };

    switch (tag) 
    {
    case eConstUtf8: value.utf8Info = utf8_info_read(scan); break;
    case eConstClass: value.classInfo = class_info_read(scan); break;
    case eConstMethodRef: value.fieldInfo = field_info_read(scan); break;
    case eConstMethodType: value.methodTypeInfo = method_type_info_read(scan); break;
    case eConstNameAndType: value.nameAndTypeInfo = name_and_type_info_read(scan); break;
    case eConstInterfaceMethodRef: value.fieldInfo = field_info_read(scan); break;
    case eConstMethodHandle: value.methodHandleInfo = method_handle_info_read(scan); break;
    case eConstInvokeDynamic: value.invokeDynamicInfo = invoke_dynamic_info_read(scan); break;
    case eConstFloat: value.floatInfo = float_info_read(scan); break;
    case eConstFieldRef: value.fieldInfo = field_info_read(scan); break;

    default: 
        NEVER("unknown tag: %s", jvm_const_tag_string(tag));
    }

    return value;
}

static void class_read(scan_t *scan)
{
    uint16_t minor = read_be16(scan);
    jvm_version_t major = read_be16(scan);

    logverbose("classfile(path=`%s`, major=`%s`, minor=%u)", scan_path(scan), jvm_version_string(major), minor);

    uint16_t constPool = read_be16(scan);
    logverbose("constPool=%u", constPool);

    for (size_t i = 0; i < constPool - 1; i++)
    {
        const_read(scan);
    }
}

void jvm_parse(driver_t *driver, scan_t *scan)
{
    lifetime_t *lifetime = handle_get_lifetime(driver);
    reports_t *reports = lifetime_get_reports(lifetime);

    uint32_t magic = read_be32(scan);
    if (magic != 0xCAFEBABE) {
        // TODO: might be a jar file
        report(reports, eWarn, NULL, "`%s` invalid magic number: 0x%x", scan_path(scan), magic);
        return;
    }

    class_read(scan);
}
