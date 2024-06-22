// SPDX-License-Identifier: GPL-3.0-only

#ifndef JVM_VERSION
#   define JVM_VERSION(id, str, it)
#endif

JVM_VERSION(eVersionJDK1_1, "JDK 1.1", 0x2D)
JVM_VERSION(eVersionJDK1_2, "JDK 1.2", 0x2E)
JVM_VERSION(eVersionJDK1_3, "JDK 1.3", 0x2F)
JVM_VERSION(eVersionJDK1_4, "JDK 1.4", 0x30)
JVM_VERSION(eVersionJava5, "Java SE 5.0", 0x31)
JVM_VERSION(eVersionJava6, "Java SE 6.0", 0x32)
JVM_VERSION(eVersionJava7, "Java SE 7", 0x33)
JVM_VERSION(eVersionJava8, "Java SE 8", 0x34)
JVM_VERSION(eVersionJava9, "Java SE 9", 0x35)
JVM_VERSION(eVersionJava10, "Java SE 10", 0x36)
JVM_VERSION(eVersionJava11, "Java SE 11", 0x37)
JVM_VERSION(eVersionJava12, "Java SE 12", 0x38)
JVM_VERSION(eVersionJava13, "Java SE 13", 0x39)
JVM_VERSION(eVersionJava14, "Java SE 14", 0x3A)
JVM_VERSION(eVersionJava15, "Java SE 15", 0x3B)
JVM_VERSION(eVersionJava16, "Java SE 16", 0x3C)
JVM_VERSION(eVersionJava17, "Java SE 17", 0x3D)
JVM_VERSION(eVersionJava18, "Java SE 18", 0x3E)
JVM_VERSION(eVersionJava19, "Java SE 19", 0x3F)
JVM_VERSION(eVersionJava20, "Java SE 20", 0x40)
JVM_VERSION(eVersionJava21, "Java SE 21", 0x41)

#undef JVM_VERSION

#ifndef JVM_CONST
#   define JVM_CONST(id, str, v)
#endif

JVM_CONST(eConstUtf8, "Utf8", 1)
JVM_CONST(eConstInt, "Int", 3)
JVM_CONST(eConstFloat, "Float", 4)
JVM_CONST(eConstLong, "Long", 5)
JVM_CONST(eConstDouble, "Double", 6)
JVM_CONST(eConstClass, "Class", 7)
JVM_CONST(eConstString, "String", 8)
JVM_CONST(eConstFieldRef, "FieldRef", 9)
JVM_CONST(eConstMethodRef, "MethodRef", 10)
JVM_CONST(eConstInterfaceMethodRef, "InterfaceMethodRef", 11)
JVM_CONST(eConstNameAndType, "NameAndType", 12)
JVM_CONST(eConstMethodHandle, "MethodHandle", 15)
JVM_CONST(eConstMethodType, "MethodType", 16)
JVM_CONST(eConstDynamic, "Dynamic", 17)
JVM_CONST(eConstInvokeDynamic, "InvokeDynamic", 18)
JVM_CONST(eConstModule, "Module", 19)
JVM_CONST(eConstPackage, "Package", 20)

#undef JVM_CONST

#ifndef JVM_ACCESS
#   define JVM_ACCESS(id, str, bits)
#endif

JVM_ACCESS(eAccessPublic, "public", 0x0001)
JVM_ACCESS(eAccessFinal, "final", 0x0010)
JVM_ACCESS(eAccessSuper, "super", 0x0020)
JVM_ACCESS(eAccessInterface, "interface", 0x0200)
JVM_ACCESS(eAccessAbstract, "abstract", 0x0400)
JVM_ACCESS(eAccessSynthetic, "synthetic", 0x1000)
JVM_ACCESS(eAccessAnnotation, "annotation", 0x2000)
JVM_ACCESS(eAccessEnum, "enum", 0x4000)
JVM_ACCESS(eAccessModule, "module", 0x8000)

#undef JVM_ACCESS

#ifndef JVM_ATTRIB
#   define JVM_ATTRIB(id, str)
#endif

// critical to interpretation of class file

JVM_ATTRIB(eAttribConstantValue, "ConstantValue")
JVM_ATTRIB(eAttribCode, "Code")
JVM_ATTRIB(eAttribStackMapTable, "StackMapTable")
JVM_ATTRIB(eAttribBootstrapMethods, "BootstrapMethods")
JVM_ATTRIB(eAttribNestHost, "NestHost")
JVM_ATTRIB(eAttribNestMembers, "NestMembers")
JVM_ATTRIB(eAttribPermittedSubclasses, "PermittedSubclasses")

// critical to interpretation of class file by class libraries

JVM_ATTRIB(eAttribExceptions, "Exceptions")
JVM_ATTRIB(eAttribInnerClasses, "InnerClasses")
JVM_ATTRIB(eAttribEnclosingMethod, "EnclosingMethod")
JVM_ATTRIB(eAttribSynthetic, "Synthetic")
JVM_ATTRIB(eAttribSignature, "Signature")
JVM_ATTRIB(eAttribRecord, "Record")
JVM_ATTRIB(eAttribSourceFile, "SourceFile")
JVM_ATTRIB(eAttribLineNumberTable, "LineNumberTable")
JVM_ATTRIB(eAttribLocalVariableTable, "LocalVariableTable")
JVM_ATTRIB(eAttribLocalVariableTypeTable, "LocalVariableTypeTable")

// metadata about class file

JVM_ATTRIB(eAttribSourceDebugExtension, "SourceDebugExtension")
JVM_ATTRIB(eAttribDeprecated, "Deprecated")
JVM_ATTRIB(eAttribRuntimeVisibleAnnotations, "RuntimeVisibleAnnotations")
JVM_ATTRIB(eAttribRuntimeInvisibleAnnotations, "RuntimeInvisibleAnnotations")
JVM_ATTRIB(eAttribRuntimeVisibleParameterAnnotations, "RuntimeVisibleParameterAnnotations")
JVM_ATTRIB(eAttribRuntimeInvisibleParameterAnnotations, "RuntimeInvisibleParameterAnnotations")
JVM_ATTRIB(eAttribRuntimeVisibleTypeAnnotations, "RuntimeVisibleTypeAnnotations")
JVM_ATTRIB(eAttribRuntimeInvisibleTypeAnnotations, "RuntimeInvisibleTypeAnnotations")
JVM_ATTRIB(eAttribAnnotationDefault, "AnnotationDefault")
JVM_ATTRIB(eAttribMethodParameters, "MethodParameters")
JVM_ATTRIB(eAttribModule, "Module")
JVM_ATTRIB(eAttribModulePackages, "ModulePackages")
JVM_ATTRIB(eAttribModuleMainClass, "ModuleMainClass")

#undef JVM_ATTRIB
