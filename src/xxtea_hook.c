#include "hook.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include "dobby.h"

/* ------------------------------------------------------------------ */
/* global state                                                        */
/* ------------------------------------------------------------------ */
static xxtea_decrypt_t    g_original_xxtea_decrypt = NULL;
static jsb_set_xxtea_key_t g_original_jsb_set_key   = NULL;
static char               g_xxtea_key[128]          = {0};
static int                g_file_index              = 0;
static pthread_mutex_t    g_mutex                   = PTHREAD_MUTEX_INITIALIZER;

/* ------------------------------------------------------------------ */
/* xxtea_decrypt hook                                                  */
/* ------------------------------------------------------------------ */
static unsigned char* hook_xxtea_decrypt(
    unsigned char* data, size_t len,
    unsigned char* key, size_t* out_len)
{
    unsigned char* result = g_original_xxtea_decrypt(data, len, key, out_len);

    if (result && out_len && *out_len > 0) {
        pthread_mutex_lock(&g_mutex);

        jshook_ensure_dir(JSHOOK_DUMP_DIR);
        int idx = g_file_index++;

        /* save decrypted output */
        char path[256];
        const char* ext = ".dec";
        if (jshook_is_text(result, *out_len)) {
            ext = jshook_guess_ext(result, *out_len);
        }
        snprintf(path, sizeof(path), "%s/%04d%s", JSHOOK_DUMP_DIR, idx, ext);
        jshook_save_file(path, result, *out_len);

        /* save info file */
        snprintf(path, sizeof(path), "%s/%04d.info", JSHOOK_DUMP_DIR, idx);
        FILE* info = fopen(path, "w");
        if (info) {
            fprintf(info, "encrypted=%zu\n", len);
            fprintf(info, "decrypted=%zu\n", *out_len);
            fprintf(info, "key=%s\n", g_xxtea_key);
            fclose(info);
        }

        /* check for replacement */
        snprintf(path, sizeof(path), "%s/%04d.bin", JSHOOK_REPLACE_DIR, idx);
        size_t rep_len = 0;
        unsigned char* rep_data = jshook_load_replace(path, &rep_len);
        if (rep_data) {
            /* allocate new output buffer */
            unsigned char* new_result = (unsigned char*)malloc(rep_len);
            if (new_result) {
                memcpy(new_result, rep_data, rep_len);
                free(rep_data);
                free(result); /* original was malloc'd by xxtea_decrypt */
                *out_len = rep_len;
                pthread_mutex_unlock(&g_mutex);
                return new_result;
            }
            free(rep_data);
        }

        pthread_mutex_unlock(&g_mutex);
    }

    return result;
}

/* ------------------------------------------------------------------ */
/* jsb_set_xxtea_key hook                                              */
/* ------------------------------------------------------------------ */
static void hook_jsb_set_xxtea_key(const void* str_obj) {
    char key_buf[128] = {0};
    jshook_extract_std_string(str_obj, key_buf, sizeof(key_buf));

    if (key_buf[0]) {
        pthread_mutex_lock(&g_mutex);
        memcpy(g_xxtea_key, key_buf, sizeof(g_xxtea_key) - 1);
        g_xxtea_key[sizeof(g_xxtea_key) - 1] = 0;

        jshook_ensure_dir(JSHOOK_DUMP_DIR);
        char path[256];
        snprintf(path, sizeof(path), "%s/xxtea_key.txt", JSHOOK_DUMP_DIR);
        FILE* fp = fopen(path, "w");
        if (fp) {
            fprintf(fp, "%s\n", g_xxtea_key);
            fclose(fp);
        }
        pthread_mutex_unlock(&g_mutex);
    }

    if (g_original_jsb_set_key)
        g_original_jsb_set_key(str_obj);
}

/* ------------------------------------------------------------------ */
/* hook initialisation                                                 */
/* ------------------------------------------------------------------ */
int jshook_hook_xxtea(void) {
    /* try dlsym for xxtea_decrypt (C function, should be exported) */
    void* addr = dlsym(RTLD_DEFAULT, "xxtea_decrypt");
    if (!addr) {
        /* lib might use -fvisibility=hidden, try RTLD_LOCAL workaround */
        void* handle = dlopen("libcocos2djs.so", RTLD_NOLOAD | RTLD_LAZY);
        if (handle) {
            addr = dlsym(handle, "xxtea_decrypt");
            dlclose(handle);
        }
    }

    if (!addr) {
        /* fallback: scan lib for xxtea_decrypt string reference */
        FILE* log = fopen(JSHOOK_DUMP_DIR "/error.log", "a");
        if (log) { fprintf(log, "xxtea_decrypt not found via dlsym\n"); fclose(log); }
        return -1;
    }

    int ret = DobbyHook(addr, (void*)hook_xxtea_decrypt, (void**)&g_original_xxtea_decrypt);
    return ret;
}

int jshook_hook_jsb_set_key(void) {
    void* addr = dlsym(RTLD_DEFAULT, JSBSET_XXTEAKEY_SYM);
    if (!addr) {
        void* handle = dlopen("libcocos2djs.so", RTLD_NOLOAD | RTLD_LAZY);
        if (handle) {
            addr = dlsym(handle, JSBSET_XXTEAKEY_SYM);
            dlclose(handle);
        }
    }

    if (!addr) {
        FILE* log = fopen(JSHOOK_DUMP_DIR "/error.log", "a");
        if (log) { fprintf(log, "jsb_set_xxtea_key not found\n"); fclose(log); }
        return -1;
    }

    int ret = DobbyHook(addr, (void*)hook_jsb_set_xxtea_key, (void**)&g_original_jsb_set_key);
    return ret;
}
