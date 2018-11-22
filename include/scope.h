#ifndef __SCOPE
#define __SCOPE
typedef struct Scope_     * Scope;
extern ANN       void   scope_init(Scope);
extern ANEW ANN Vector scope_get(const Scope);
extern ANN vtype  scope_lookup0(const Scope, const vtype);
extern ANN vtype  scope_lookup1(const Scope, const vtype);
extern ANN vtype  scope_lookup2(const Scope, const vtype);
extern ANN       void   scope_add(const Scope, const vtype, const vtype);
extern ANN       void   scope_commit(const Scope);
extern ANN       void   scope_release(Scope);
ANN static inline void scope_push(const Scope s) {
  vector_add((Vector)s, (vtype)new_map());
}

ANN static inline void scope_pop(const Scope s) {
  free_map((Map)vector_pop((Vector)s));
}
#endif