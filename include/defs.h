/** @file: defs.h
\brief base definitions
**/
#ifndef __DEF
#define __DEF

#ifdef USE_GETTEXT
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#define _(String) dgettext(GWION_PACKAGE, String)
#else
#define _(String) (String)
#endif

#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(ptr)-offsetof(type, member)))

#define GW_ERROR   -1
#define GW_OK      1
#define MEM_STEP   16
#define SIZEOF_MEM (0x1 << MEM_STEP)
#define SIZEOF_REG (0x1 << 14)

#define ANN       __attribute__((nonnull))
#define ANN2(...) __attribute__((nonnull(__VA_ARGS__)))
//#define ANEW __attribute__((returns_nonnull,malloc))
#define ANEW  __attribute__((malloc))
#define NUSED __attribute__((unused))

#ifdef __GNUC__
#ifdef __clang__
#define LOOP_OPTIM
#define CC_OPTIM(a)
#else
#define LOOP_OPTIM  _Pragma("GCC ivdep")
#define CC_OPTIM(a) __attribute__((optimize(#a)))
#endif
#else
#define LOOP_OPTIM
#endif

// maybe later enclose in ifdef
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define CHECK_BB(f)                                                            \
  do {                                                                         \
    if (f < 0) return GW_ERROR;                                                \
  } while (0)
#define CHECK_OB(f)                                                            \
  do {                                                                         \
    if (!f) return GW_ERROR;                                                   \
  } while (0)
#define CHECK_BO(f)                                                            \
  do {                                                                         \
    if (f < 0) return NULL;                                                    \
  } while (0)
#define CHECK_OO(f)                                                            \
  do {                                                                         \
    if (!f) return NULL;                                                       \
  } while (0)

#define DECL_BB(decl, f, exp)                                                  \
  decl f exp;                                                                  \
  if (f < 0) return GW_ERROR
#define DECL_OB(decl, f, exp)                                                  \
  decl f exp;                                                                  \
  if (!f) return GW_ERROR
#define DECL_BO(decl, f, exp)                                                  \
  decl f exp;                                                                  \
  if (f < 0) return NULL
#define DECL_OO(decl, f, exp)                                                  \
  decl f exp;                                                                  \
  if (!f) return NULL

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "gwcommon.h"

typedef unsigned int uint;

static inline m_uint num_digit(const m_uint i) {
  return i ? (m_uint)floor(log10((float)i) + 1) : 1;
}

static inline m_uint round2szint(const m_uint i) {
  return ((i + (SZ_INT - 1)) & ~(SZ_INT - 1));
}
#endif
