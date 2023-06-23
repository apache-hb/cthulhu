#ifndef ENDIAN
#   define ENDIAN(id, name, v)
#endif

ENDIAN(eEndianNative, "native", BYTE_ORDER)
ENDIAN(eEndianLittle, "little", LITTLE_ENDIAN)
ENDIAN(eEndianBig, "big", BIG_ENDIAN)

#undef ENDIAN
