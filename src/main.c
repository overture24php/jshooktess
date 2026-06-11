/* ------------------------------------------------------------------ */
/* JSHook for Cocos2d-js                                              */
/* Dobby hook library untuk intercept XXTEA decryption                */
/* Output: /sdcard/jshook_dump/                                       */
/* Replace: /sdcard/jshook_replace/                                   */
/* ------------------------------------------------------------------ */
#include "hook.h"
#include "utils.h"
#include "dobby.h"
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>

/* forward declarations from xxtea_hook.c */
int jshook_hook_xxtea(void);
int jshook_hook_jsb_set_key(void);

/* hook for cocos2d::FileUtils::getStringFromFile - optional but useful */
/* This hooks the file loading to track which file is being decrypted */

/* ---- cocos2d::FileUtils::getStringFromFile hook ---- */
typedef void* (*getStringFromFile_t)(void* fileUtils, const void* path_str);
static getStringFromFile_t g_original_getStringFromFile = NULL;

static void* hook_getStringFromFile(void* fileUtils, const void* path_str) {
    char path_buf[256] = {0};
    jshook_extract_std_string(path_str, path_buf, sizeof(path_buf));

    /* log file access */
    FILE* log = fopen(JSHOOK_DUMP_DIR "/files.log", "a");
    if (log) {
        fprintf(log, "getStringFromFile: %s\n", path_buf);
        fclose(log);
    }

    return g_original_getStringFromFile(fileUtils, path_str);
}

/* ---- cocos2d::FileUtils::getDataFromFile hook ---- */
typedef void* (*getDataFromFile_t)(void* fileUtils, const void* path_str);
static getDataFromFile_t g_original_getDataFromFile = NULL;

static void* hook_getDataFromFile(void* fileUtils, const void* path_str) {
    char path_buf[256] = {0};
    jshook_extract_std_string(path_str, path_buf, sizeof(path_buf));

    FILE* log = fopen(JSHOOK_DUMP_DIR "/files.log", "a");
    if (log) {
        fprintf(log, "getDataFromFile: %s\n", path_buf);
        fclose(log);
    }

    return g_original_getDataFromFile(fileUtils, path_str);
}

/* try to hook FileUtils methods (C++ mangled names vary by NDK version) */
static void try_hook_file_utils(void) {
    /* Common mangled names for different NDK versions */
    const char* symbols[] = {
        GETSTRINGFROMFILE_SYM,
        GETDATAFROMFILE_SYM,
        NULL
    };

    for (int i = 0; symbols[i]; i++) {
        void* addr = dlsym(RTLD_DEFAULT, symbols[i]);
        if (!addr) {
            void* handle = dlopen("libcocos2djs.so", RTLD_NOLOAD | RTLD_LAZY);
            if (handle) {
                addr = dlsym(handle, symbols[i]);
                dlclose(handle);
            }
        }
        if (addr) {
            if (i == 0)
                DobbyHook(addr, (void*)hook_getStringFromFile, (void**)&g_original_getStringFromFile);
            else
                DobbyHook(addr, (void*)hook_getDataFromFile, (void**)&g_original_getDataFromFile);
        }
    }
}

/* ---- library entry point ---- */
__attribute__((constructor)) static void jshook_init(void) {
    /* ensure dump directory */
    jshook_ensure_dir(JSHOOK_DUMP_DIR);
    jshook_ensure_dir(JSHOOK_REPLACE_DIR);

    /* log init */
    FILE* log = fopen(JSHOOK_DUMP_DIR "/init.log", "w");
    if (log) {
        fprintf(log, "JSHook for Cocos2d-js loaded\n");
        fprintf(log, "PID: %d\n", getpid());
        fclose(log);
    }

    /* hook xxtea_decrypt */
    if (jshook_hook_xxtea() == 0) {
        log = fopen(JSHOOK_DUMP_DIR "/init.log", "a");
        if (log) { fprintf(log, "xxtea_decrypt: hooked OK\n"); fclose(log); }
    } else {
        log = fopen(JSHOOK_DUMP_DIR "/init.log", "a");
        if (log) { fprintf(log, "xxtea_decrypt: hook FAILED\n"); fclose(log); }
    }

    /* hook jsb_set_xxtea_key */
    if (jshook_hook_jsb_set_key() == 0) {
        log = fopen(JSHOOK_DUMP_DIR "/init.log", "a");
        if (log) { fprintf(log, "jsb_set_xxtea_key: hooked OK\n"); fclose(log); }
    } else {
        log = fopen(JSHOOK_DUMP_DIR "/init.log", "a");
        if (log) { fprintf(log, "jsb_set_xxtea_key: hook FAILED\n"); fclose(log); }
    }

    /* try hooking file utils (optional, for file path tracking) */
    try_hook_file_utils();

    log = fopen(JSHOOK_DUMP_DIR "/init.log", "a");
    if (log) { fprintf(log, "JSHook init complete\n"); fclose(log); }
}

/* JNI_OnLoad for compatibility with injectors that use JNI */
int JNI_OnLoad(void* vm, void* reserved) {
    /* init already runs via constructor, this is just for compat */
    (void)vm; (void)reserved;
    return 0x00010006; /* JNI_VERSION_1_6 */
}
