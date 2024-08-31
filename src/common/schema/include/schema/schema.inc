// SPDX-License-Identifier: LGPL-3.0-only

#ifndef SCHEMA_STATUS
#   define SCHEMA_STATUS(id, name)
#endif

SCHEMA_STATUS(eSchemaStatusOk, "ok")

#undef SCHEMA_STATUS

#ifndef SCHEMA_TYPE
#   define SCHEMA_TYPE(id, name)
#endif

SCHEMA_TYPE(eSchemaTypeString, "string")
SCHEMA_TYPE(eSchemaTypePointer, "pointer")
SCHEMA_TYPE(eSchemaTypeStruct, "struct")
SCHEMA_TYPE(eSchemaTypeUnion, "union")

#undef SCHEMA_TYPE
