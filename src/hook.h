#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* xxtea_decrypt - exported from libcocos2djs.so (C function) */
typedef unsigned char* (*xxtea_decrypt_t)(
    unsigned char* data, size_t len,
    unsigned char* key, size_t* out_len
);

/* jsb_set_xxtea_key(const std::string&) - C++ mangled */
typedef void (*jsb_set_xxtea_key_t)(const void* std_string);

/* dump directories */
#define JSHOOK_DUMP_DIR    "/sdcard/jshook_dump"
#define JSHOOK_REPLACE_DIR "/sdcard/jshook_replace"

/* mangled symbol for jsb_set_xxtea_key (libc++ on android) */
#define JSBSET_XXTEAKEY_SYM \
    "_Z17jsb_set_xxtea_keyRKNSt6__ndk112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE"

/* mangled symbols for FileUtils (verified from .dynsym) */
#define GETDATAFROMFILE_SYM \
    "_ZN7cocos2d9FileUtils15getDataFromFileERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE"
#define GETSTRINGFROMFILE_SYM \
    "_ZN7cocos2d9FileUtils17getStringFromFileERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE"

#ifdef __cplusplus
}
#endif
