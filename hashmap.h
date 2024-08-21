/*
   The latest version of this library is available on GitHub;
   https://github.com/sheredom/hashmap.h
*/

/*
   This is free and unencumbered software released into the public domain.

   Anyone is free to copy, modify, publish, use, compile, sell, or
   distribute this software, either in source code form or as a compiled
   binary, for any purpose, commercial or non-commercial, and by any
   means.

   In jurisdictions that recognize copyright laws, the author or authors
   of this software dedicate any and all copyright interest in the
   software to the public domain. We make this dedication for the benefit
   of the public at large and to the detriment of our heirs and
   successors. We intend this dedication to be an overt act of
   relinquishment in perpetuity of all present and future rights to this
   software under copyright law.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.

   For more information, please refer to <http://unlicense.org/>
*/
#ifndef SHEREDOM_HASHMAP_H_INCLUDED
#define SHEREDOM_HASHMAP_H_INCLUDED

#if defined(_MSC_VER)
// Workaround a bug in the MSVC runtime where it uses __cplusplus when not
// defined.
#pragma warning(push, 0)
#pragma warning(disable : 4668)
#endif

#include <stdlib.h>
#include <string.h>

#if (defined(_MSC_VER) && defined(__AVX__)) ||                                 \
    (!defined(_MSC_VER) && defined(__SSE4_2__))
#define HASHMAP_X86_SSE42
#endif

#if defined(HASHMAP_X86_SSE42)
#include <nmmintrin.h>
#endif

#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32)
#define HASHMAP_ARM_CRC32
#endif

#if defined(HASHMAP_ARM_CRC32)
#include <arm_acle.h>
#endif

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(_MSC_VER)
#pragma warning(push)
/* Stop MSVC complaining about unreferenced functions */
#pragma warning(disable : 4505)
/* Stop MSVC complaining about not inlining functions */
#pragma warning(disable : 4710)
/* Stop MSVC complaining about inlining functions! */
#pragma warning(disable : 4711)
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wstatic-in-inline"

#if __has_warning("-Wunsafe-buffer-usage")
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#endif

#if defined(__TINYC__)
#define HASHMAP_ATTRIBUTE(a) __attribute((a))
#else
#define HASHMAP_ATTRIBUTE(a) __attribute__((a))
#endif

#if defined(_MSC_VER)
#define HASHMAP_WEAK __inline
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define HASHMAP_WEAK static HASHMAP_ATTRIBUTE(used)
#elif defined(__clang__) || defined(__GNUC__) || defined(__TINYC__)
#define HASHMAP_WEAK HASHMAP_ATTRIBUTE(weak)
#else
#error Non clang, non gcc, non MSVC, non tcc compiler found!
#endif

#if defined(_MSC_VER)
#define HASHMAP_ALWAYS_INLINE __forceinline
#elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) ||          \
    defined(__cplusplus)
#define HASHMAP_ALWAYS_INLINE __attribute__((always_inline)) inline
#else
/* If we cannot use inline, its not safe to use always_inline, so we mark the
 * function weak. */
#define HASHMAP_ALWAYS_INLINE HASHMAP_WEAK
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1920)
typedef unsigned __int8 hashmap_uint8_t;
typedef unsigned __int32 hashmap_uint32_t;
typedef unsigned __int64 hashmap_uint64_t;
#else
#include <stdint.h>
typedef uint8_t hashmap_uint8_t;
typedef uint32_t hashmap_uint32_t;
typedef uint64_t hashmap_uint64_t;
#endif

typedef struct hashmap_element_s {
  const void *key;
  hashmap_uint32_t key_len;
  int in_use;
  void *data;
} hashmap_element_t;

typedef hashmap_uint32_t (*hashmap_hasher_t)(hashmap_uint32_t seed,
                                             const void *key,
                                             hashmap_uint32_t key_len);
typedef int (*hashmap_comparer_t)(const void *a, hashmap_uint32_t a_len,
                                  const void *b, hashmap_uint32_t b_len);

typedef struct hashmap_s {
  hashmap_uint32_t log2_capacity;
  hashmap_uint32_t size;
  hashmap_hasher_t hasher;
  hashmap_comparer_t comparer;
  struct hashmap_element_s *data;
} hashmap_t;

#define HASHMAP_LINEAR_PROBE_LENGTH (8)

typedef struct hashmap_create_options_s {
  hashmap_hasher_t hasher;
  hashmap_comparer_t comparer;
  hashmap_uint32_t initial_capacity;
  hashmap_uint32_t _;
} hashmap_create_options_t;

#if defined(__cplusplus)
extern "C" {
#endif

/// @brief Create a hashmap.
/// @param initial_capacity The initial capacity of the hashmap.
/// @param out_hashmap The storage for the created hashmap.
/// @return On success 0 is returned.
HASHMAP_WEAK int hashmap_create(const hashmap_uint32_t initial_capacity,
                                struct hashmap_s *const out_hashmap);

/// @brief Create a hashmap.
/// @param options The options to create the hashmap with.
/// @param out_hashmap The storage for the created hashmap.
/// @return On success 0 is returned.
///
/// The options members work as follows:
/// - initial_capacity The initial capacity of the hashmap.
/// - hasher Which hashing function to use with the hashmap (by default the
//    crc32 with Robert Jenkins' mix is used).
HASHMAP_WEAK int hashmap_create_ex(struct hashmap_create_options_s options,
                                   struct hashmap_s *const out_hashmap);

/// @brief Put an element into the hashmap.
/// @param hashmap The hashmap to insert into.
/// @param key The string key to use.
/// @param len The length of the string key.
/// @param value The value to insert.
/// @return On success 0 is returned.
///
/// The key string slice is not copied when creating the hashmap entry, and thus
/// must remain a valid pointer until the hashmap entry is removed or the
/// hashmap is destroyed.
HASHMAP_WEAK int hashmap_put(struct hashmap_s *const hashmap,
                             const void *const key, const hashmap_uint32_t len,
                             void *const value);

/// @brief Get an element from the hashmap.
/// @param hashmap The hashmap to get from.
/// @param key The string key to use.
/// @param len The length of the string key.
/// @return The previously set element, or NULL if none exists.
HASHMAP_WEAK void *hashmap_get(const struct hashmap_s *const hashmap,
                               const void *const key,
                               const hashmap_uint32_t len);

/// @brief Remove an element from the hashmap.
/// @param hashmap The hashmap to remove from.
/// @param key The string key to use.
/// @param len The length of the string key.
/// @return On success 0 is returned.
HASHMAP_WEAK int hashmap_remove(struct hashmap_s *const hashmap,
                                const void *const key,
                                const hashmap_uint32_t len);

/// @brief Remove an element from the hashmap.
/// @param hashmap The hashmap to remove from.
/// @param key The string key to use.
/// @param len The length of the string key.
/// @return On success the original stored key pointer is returned, on failure
/// NULL is returned.
HASHMAP_WEAK const void *
hashmap_remove_and_return_key(struct hashmap_s *const hashmap,
                              const void *const key,
                              const hashmap_uint32_t len);

/// @brief Iterate over all the elements in a hashmap.
/// @param hashmap The hashmap to iterate over.
/// @param iterator The function pointer to call on each element.
/// @param context The context to pass as the first argument to f.
/// @return If the entire hashmap was iterated then 0 is returned. Otherwise if
/// the callback function f returned non-zero then non-zero is returned.
HASHMAP_WEAK int hashmap_iterate(const struct hashmap_s *const hashmap,
                                 int (*iterator)(void *const context,
                                                 void *const value),
                                 void *const context);

/// @brief Iterate over all the elements in a hashmap.
/// @param hashmap The hashmap to iterate over.
/// @param iterator The function pointer to call on each element.
/// @param context The context to pass as the first argument to f.
/// @return If the entire hashmap was iterated then 0 is returned.
/// Otherwise if the callback function f returned positive then the positive
/// value is returned.  If the callback function returns -1, the current item
/// is removed and iteration continues.
HASHMAP_WEAK int hashmap_iterate_pairs(
    struct hashmap_s *const hashmap,
    int (*iterator)(void *const, struct hashmap_element_s *const),
    void *const context);

/// @brief Get the size of the hashmap.
/// @param hashmap The hashmap to get the size of.
/// @return The size of the hashmap.
HASHMAP_ALWAYS_INLINE hashmap_uint32_t
hashmap_num_entries(const struct hashmap_s *const hashmap);

/// @brief Get the capacity of the hashmap.
/// @param hashmap The hashmap to get the size of.
/// @return The capacity of the hashmap.
HASHMAP_ALWAYS_INLINE hashmap_uint32_t
hashmap_capacity(const struct hashmap_s *const hashmap);

/// @brief Destroy the hashmap.
/// @param hashmap The hashmap to destroy.
HASHMAP_WEAK void hashmap_destroy(struct hashmap_s *const hashmap);

static hashmap_uint32_t hashmap_crc32_hasher(const hashmap_uint32_t seed,
                                             const void *const s,
                                             const hashmap_uint32_t len);
static int hashmap_memcmp_comparer(const void *const a,
                                   const hashmap_uint32_t a_len,
                                   const void *const b,
                                   const hashmap_uint32_t b_len);
HASHMAP_ALWAYS_INLINE hashmap_uint32_t hashmap_hash_helper_int_helper(
    const struct hashmap_s *const m, const void *const key,
    const hashmap_uint32_t len);
HASHMAP_ALWAYS_INLINE int
hashmap_hash_helper(const struct hashmap_s *const m, const void *const key,
                    const hashmap_uint32_t len,
                    hashmap_uint32_t *const out_index);
HASHMAP_WEAK int hashmap_rehash_iterator(void *const new_hash,
                                         struct hashmap_element_s *const e);
HASHMAP_ALWAYS_INLINE int hashmap_rehash_helper(struct hashmap_s *const m);
HASHMAP_ALWAYS_INLINE hashmap_uint32_t hashmap_clz(const hashmap_uint32_t x);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
#define HASHMAP_CAST(type, x) static_cast<type>(x)
#define HASHMAP_PTR_CAST(type, x) reinterpret_cast<type>(x)
#define HASHMAP_NULL NULL
#else
#define HASHMAP_CAST(type, x) ((type)(x))
#define HASHMAP_PTR_CAST(type, x) ((type)(x))
#define HASHMAP_NULL 0
#endif


#endif
