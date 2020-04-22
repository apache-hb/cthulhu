#define VECTOR_NAME string
#define VECTOR_TYPE const char*
#include "vector.h"

typedef enum {
    /* target languages */
    TargetLangC,
    TargetLangCPP,
    TargetLangLLVM,
    TargetLangASM,
    TargetLangNative
} TargetLang;

typedef enum {
    LangfeaturesWerror = 1 << 0
} LangFeatures;

typedef struct {
    int argc;
    const char** argv;

    /* warning level */
    int warning;

    /* features */
    LangFeatures features;

    /* output name */
    const char* out;

    /* input source files */
    vec_string_struct sources;

    /* input libs */
    vec_string_struct libs;

    /* target language */
    TargetLang target;
} ArgData;

ArgData ArgParse(int argc, const char** argv)
{
    ArgData data;
    int i;
    i = 1;

    data.argc = argc;
    data.argv = argv;
    data.features = 0;
    data.target = TargetLangNative;
    data.out = NULL;
    data.warning = 1;

    vec_string_init(&data.sources);
    vec_string_init(&data.libs);

    while(i < argc)
    {
        if(strcmp(argv[i], "--target") == 0)
        {
            i++;
            if(strcmp(argv[i], "c") == 0)
            {
                data.target = TargetLangC;
            }
            else if(strcmp(argv[i], "cpp") == 0)
            {
                data.target = TargetLangCPP;
            }
            else if(strcmp(argv[i], "llvm") == 0)
            {
                data.target = TargetLangLLVM;
            }
            else if(strcmp(argv[i], "asm") == 0)
            {
                data.target = TargetLangASM;
            }
            else
            {
                printf("invalid target %s\n", argv[i]);
            }
        }
        else if(strcmp(argv[i], "--out") == 0)
        {
            data.out = argv[++i];
        }
        else if(strcmp(argv[i], "--link") == 0)
        {
            vec_string_append(&data.libs, argv[++i]);
        }
        else if(strcmp(argv[i], "-w0") == 0)
        {
            data.warning = 0;
        }
        else if(strcmp(argv[i], "-w1") == 0)
        {
            data.warning = 1;
        }
        else if(strcmp(argv[i], "-w2") == 0)
        {
            data.warning = 2;
        }
        else if(strcmp(argv[i], "-w3") == 0)
        {
            data.warning = 3;
        }
        else if(strcmp(argv[i], "-w4") == 0)
        {
            data.warning = 4;
        }
        else if(strcmp(argv[i], "-werror") == 0)
        {
            data.features |= LangfeaturesWerror;
        }
        else
        {
            vec_string_append(&data.sources, argv[i]);
        }

        i++;
    }

    if(!data.out)
    {
        if(data.target == TargetLangNative)
        {
#if defined(_WIN32)
            data.out = "exec.exe";
#elif defined(__APPLE__) && defined(__MACH__)
            data.out = "exec.out";
#elif defined(__linux__)
            data.out = "exec.out";
#endif
        }
        else if(data.target == TargetLangASM)
        {
            data.out = "exec.asm";
        }
        else if(data.target == TargetLangC)
        {
            data.out = "exec.c";
        }
        else if(data.target == TargetLangCPP)
        {
            data.out = "exec.cpp";
        }
        else if(data.target == TargetLangLLVM)
        {
            /* TODO: what is the file extension for llvm bytecode? */
            data.out = "exec.x";
        }
    }

    return data;
}