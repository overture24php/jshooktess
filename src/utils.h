#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ensure directory exists (mkdir -p equivalent) */
void jshook_ensure_dir(const char* path);

/* save data to file */
int jshook_save_file(const char* path, const unsigned char* data, size_t len);

/* check if file exists and load it (for replacement) */
unsigned char* jshook_load_replace(const char* path, size_t* out_len);

/* extract std::string content from libc++ std::string object */
void jshook_extract_std_string(const void* str_obj, char* out, size_t out_size);

/* memory pattern scan */
void* jshook_find_pattern(const char* pattern, const char* mask, size_t len);

/* detect if data is text vs binary */
int jshook_is_text(const unsigned char* data, size_t len);

/* get file extension hint based on first bytes */
const char* jshook_guess_ext(const unsigned char* data, size_t len);

#ifdef __cplusplus
}
#endif
