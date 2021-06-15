/* test runner */

#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#   include <dirent.h>
#   include <sys/wait.h>
#   define SEP "/"
#else
#   define WIN32_MEAN_AND_LEAN
    /* now isnt this a fun macro */
#   define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
#   include <windows.h>
#   include <tchar.h>
#   define SEP "\\"
#endif

#define COLOUR_RED "\x1B[1;31m"
#define COLOUR_GREEN "\x1B[0;32m"
#define COLOUR_YELLOW "\x1B[0;33m"
#define COLOUR_RESET "\x1B[0m"

static const char *driver = NULL;
static const char *path = NULL;

static int fails = 0;

static char *get_cmd(const char *name) {
    char *buffer = malloc(0x1000);
    sprintf(buffer, "%s %s" SEP "%s --target=gcc-x64", driver, path, name);
    return buffer;
}

static int run_cmd(const char *cmd) {
#ifndef _WIN32
    return WEXITSTATUS(system(cmd));
#else
    return system(cmd);
#endif
}

static void run_test(const char *pass, const char *cmd, int res) {
    int err = run_cmd(cmd);
    if (err != res) {
        fails += 1;
        fprintf(stderr, COLOUR_RED "\t%s failed! (%d)\n" COLOUR_RESET, pass, err);
    } else {
        printf(COLOUR_GREEN "\t%s passed!\n" COLOUR_RESET, pass);
    }
}

static void dir_iter(const char *dir, void(*func)(const char*)) {
#ifndef _WIN32
    struct dirent *entry;
    DIR *iter = opendir(dir);
    if (iter == NULL) {
        fprintf(stderr, "`%s` doesnt exist\n", path);
        fails += 1;
        return;
    }

    while ((entry = readdir(iter)) != NULL) {
        const char *name = entry->d_name;
        func(name);
    }

    closedir(iter);
#else
    WIN32_FIND_DATA find;
    HANDLE handle = FindFirstFile(dir, &find);

    do {
        if (!(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            _tprintf(TEXT("%s\n"), find.cFileName);
            func(find.cFileName);
        }
    } while (FindNextFile(handle, &find));

    FindClose(handle);
#endif
}

static void run_test_and_exec(const char *name) {
    if (name[0] != '.') {
        printf(COLOUR_YELLOW "test:" COLOUR_RESET " %s\n", name);
        run_test("compile", get_cmd(name), 0);
        run_test("run", "./a.out", 1);
    }
}

int main(int argc, const char **argv) {
    if (3 > argc) {
        fprintf(stderr, "usage: %s <dir> <driver>\n", argv[0]);
        return 1;
    }

    path = argv[1];
    driver = argv[2];

    dir_iter(path, run_test_and_exec);

    return !!fails;
}
