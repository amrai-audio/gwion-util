#include "gwion_util.h"
#ifdef USE_HELGRIND
#include "valgrind/helgrind.h"
#endif

#define SMALL_BLK   512
#define BIG_BLK     16
#define HUGE        (64 * SZ_INT)
#define BLK(obj_sz) (obj_sz < HUGE ? SMALL_BLK : BIG_BLK)

struct Recycle {
  volatile struct Recycle *next;
};

struct pool {
  uint8_t **      data;
  MUTEX_TYPE mutex;
  volatile struct Recycle *next;
  uint32_t        obj_sz;
  uint32_t        obj_id;
  int32_t    blk_id;
  uint32_t   nblk;
};

ANN static void mp_set(struct pool *p, const uint32_t obj_sz) {
  p->obj_sz = obj_sz;
  p->obj_id = BLK(obj_sz) - 1;
  p->blk_id = -1;
  p->nblk   = 1;
  p->next   = NULL;
  p->data   = (uint8_t **)xcalloc(1, sizeof(uint8_t *));
  MUTEX_SETUP(p->mutex);
}

MemPool mempool_ini(const size_t sz) {
  MemPool p      = (MemPool)xmalloc(sizeof(struct MemPool_));
  p->master_pool = new_pool(sizeof(struct pool));
  p->sizes       = xmalloc((log2(sz) - 1) * sizeof(size_t));
  p->sz          = 0;
  for (size_t j = SZ_INT, k = 0; sz >= k; k = j, j <<= 1) p->sizes[p->sz++] = j;
  p->pools = (struct pool **)xcalloc(p->sz, sizeof(struct pool *));
  return p;
}

void mempool_end(MemPool mp) {
  LOOP_OPTIM
  for (m_uint i = mp->sz + 1; --i;) {
    struct pool *p = mp->pools[i - 1];
    if (p) mp_end(p);
  }
  xfree(mp->sizes);
  mp_end(mp->master_pool);
  xfree(mp->master_pool);
  xfree(mp->pools);
  xfree(mp);
}

static inline struct pool *mp_create(MemPool mp, const uint32_t obj_sz,
                                     const uint32_t idx) {
  struct pool *p = (struct pool *)_mp_calloc2(mp->master_pool, 0);
  mp_set(p, obj_sz);
  mp->pools[idx] = p;
  return p;
}

ANN struct pool *mp_ini(MemPool mp, const uint32_t obj_sz) {
  for (size_t i = 0; i < mp->sz; ++i) {
    if (obj_sz <= mp->sizes[i])
      return mp->pools[i] ?: mp_create(mp, mp->sizes[i], i);
  }
  return NULL;
}

void mp_end(struct pool *p) {
  MUTEX_CLEANUP(p->mutex);
  for (uint32_t i = 0; i < p->nblk && p->data[i]; ++i) xfree(p->data[i]);
  xfree(p->data);
}

static void _realloc(struct pool *p) {
  p->obj_id = 0;
  if (++p->blk_id == (int32_t)p->nblk) {
    const uint32_t nblk = p->nblk + 1;
    p->nblk <<= 1;
    p->data = (uint8_t **)xrealloc(p->data, sizeof(uint8_t *) * p->nblk);
    for (uint32_t i = nblk; i < p->nblk; ++i)
      p->data[i] = NULL;
  }
  p->data[p->blk_id] = (uint8_t *)xcalloc(BLK(p->obj_sz), p->obj_sz);
}

static void *__mp_calloc2(struct pool *p, const m_bool zero) {
  if (p->next) {
    volatile struct Recycle *const recycle = p->next;
    #ifdef USE_HELGRIND
    VALGRIND_HG_CLEAN_MEMORY(recycle, p->obj_sz);
    #endif
    p->next                       = p->next->next;
    if (zero) memset((void*)recycle, 0, p->obj_sz);
    return (void*)recycle;
  }
  if (++p->obj_id == BLK(p->obj_sz)) _realloc(p);
  return p->data[p->blk_id] + p->obj_id * p->obj_sz;
}

void *_mp_calloc2(struct pool *p, const m_bool zero) {
  MUTEX_LOCK(p->mutex);
  void *ret = __mp_calloc2(p, zero);
  MUTEX_UNLOCK(p->mutex);
  return ret;
}

void _mp_free2(struct pool *p, void *ptr) {
  MUTEX_LOCK(p->mutex);
  volatile struct Recycle *next = p->next;
#ifdef POOL_CHECK
  memset(ptr, 0, p->obj_sz);
#endif
  p->next       = ptr;
  p->next->next = next;
  MUTEX_UNLOCK(p->mutex);
}

void _mp_free(MemPool mp, const m_uint size, void *ptr) {
  struct pool *p = mp_ini(mp, size);
  if (p)
    _mp_free2(p, ptr);
  else
    xfree(ptr);
}

struct pool *new_pool(const uint32_t obj_sz) {
  struct pool *p = (struct pool *)xmalloc(sizeof(struct pool));
  mp_set(p, obj_sz);
  return p;
}

#define MP_ALLOC(name, zero, default)                                          \
  void *_mp_##name(MemPool mp, const m_uint size) {                            \
    struct pool *p   = mp_ini(mp, size);                                       \
    void *       ret = p ? _mp_calloc2(p, zero) : (void *)default;             \
    return ret;                                                                \
  }

MP_ALLOC(malloc, 0, xmalloc(size))
MP_ALLOC(calloc, 1, xcalloc(1, size))

void *mp_realloc(MemPool mp, void *ptr, const m_uint curr, const m_uint next) {
  void *ret = _mp_malloc(mp, next);
  if (ret != ptr) memcpy(ret, ptr, curr);
  mp_free2(mp, curr, ptr);
  return ret;
}
