#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <link.h>

void jshook_ensure_dir(const char* path) {
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0777);
            *p = '/';
        }
    }
    mkdir(tmp, 0777);
}

int jshook_save_file(const char* path, const unsigned char* data, size_t len) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return -1;
    size_t wrote = fwrite(data, 1, len, fp);
    fclose(fp);
    return (wrote == len) ? 0 : -1;
}

unsigned char* jshook_load_replace(const char* path, size_t* out_len) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    if (len <= 0) { fclose(fp); return NULL; }
    fseek(fp, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc((size_t)len);
    if (!buf) { fclose(fp); return NULL; }
    size_t read = fread(buf, 1, (size_t)len, fp);
    fclose(fp);
    if (read != (size_t)len) { free(buf); return NULL; }
    *out_len = (size_t)len;
    return buf;
}

/* libc++ std::string layout:
 *   struct { char* __data_; size_t __size_; size_t __cap_; }
 *   For SSO (short string): __data_ points to inline buffer inside __cap_ area
 *   We just read __data_ and __size_ directly
 */
void jshook_extract_std_string(const void* str_obj, char* out, size_t out_size) {
    if (!str_obj || !out || out_size < 1) return;
    out[0] = 0;

    const char** data_ptr = (const char**)str_obj;
    const size_t* size_ptr = (const size_t*)str_obj + 1;
    const size_t* cap_ptr  = (const size_t*)str_obj + 2;

    const char* d = *data_ptr;
    size_t sz = *size_ptr;
    size_t cap = *cap_ptr;

    if (sz == 0) return;

    /* SSO check: libc++ sets lowest bit of capacity for long string */
    if ((cap & 1) == 0) {
        /* short string: data is inline (within the object itself) */
        d = (const char*)str_obj;
    }

    if (!d) return;
    if ((uintptr_t)d < 0x1000) return; /* invalid pointer */
    if (sz >= out_size) sz = out_size - 1;

    memcpy(out, d, sz);
    out[sz] = 0;
}

int jshook_is_text(const unsigned char* data, size_t len) {
    size_t check = (len > 512) ? 512 : len;
    for (size_t i = 0; i < check; i++) {
        unsigned char c = data[i];
        if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
            return 0; /* binary */
        }
    }
    return 1; /* likely text */
}

const char* jshook_guess_ext(const unsigned char* data, size_t len) {
    if (len == 0) return ".bin";
    /* skip whitespace */
    size_t start = 0;
    while (start < len && (data[start] == ' ' || data[start] == '\n' || data[start] == '\r' || data[start] == '\t'))
        start++;
    if (start >= len) return ".txt";

    unsigned char c = data[start];
    if (c == '{' || c == '[') return ".json";
    if (c == '(' || c == '/' || c == '"' || c == '\'') return ".js";
    if (c == '<') return ".xml";
    return ".txt";
}

/* simple pattern scanner: find bytes sequence in memory of loaded libraries */
void* jshook_find_pattern(const char* pattern, const char* mask, size_t len) {
    /* Scan in libcocos2djs.so */
    struct link_map* lm = NULL;
    void* handle = dlopen("libcocos2djs.so", RTLD_NOLOAD);
    if (!handle) return NULL;

    /* get link_map from dladdr */
    Dl_info info;
    if (dladdr((void*)jshook_find_pattern, &info)) {
        /* We know which lib we're in, scan from start of libcocos2djs */
    }

    return NULL; /* TODO: implement full pattern scan */
}
