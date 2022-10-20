#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "merge.h"
#include "util/debug.h"
#include "util/hashmap.h"
#include "util/xmalloc.h"
#include "util/xsnprintf.h"

enum {
    FIXBUF_SIZE = 512
};

static const char *fix_name(char *buf, const char *prefix, const char *name)
{
    size_t plen = strlen(prefix);
    size_t nlen = strlen(name);
    if (unlikely(size_add(plen, nlen) >= FIXBUF_SIZE)) {
        fatal_error(__func__, ENOBUFS);
    }
    memcpy(buf, prefix, plen);
    memcpy(buf + plen, name, nlen + 1);
    return buf;
}

static void fix_action(const Syntax *syn, Action *a, const char *prefix, char *buf)
{
    if (a->destination) {
        const char *name = fix_name(buf, prefix, a->destination->name);
        a->destination = find_state(syn, name);
    }
    if (a->emit_name) {
        a->emit_name = xstrdup(a->emit_name);
    }
}

static void fix_conditions (
    const Syntax *syn,
    State *s,
    const SyntaxMerge *m,
    const char *prefix,
    char *buf
) {
    for (size_t i = 0, n = s->conds.count; i < n; i++) {
        Condition *c = s->conds.ptrs[i];
        fix_action(syn, &c->a, prefix, buf);
        if (!c->a.destination && cond_type_has_destination(c->type)) {
            c->a.destination = m->return_state;
        }

        if (m->delim && c->type == COND_HEREDOCEND) {
            c->u.heredocend.data = xmemdup(m->delim, m->delim_len);
            c->u.heredocend.length = m->delim_len;
        }
    }

    fix_action(syn, &s->default_action, prefix, buf);
    if (!s->default_action.destination) {
        s->default_action.destination = m->return_state;
    }
}

// Merge a sub-syntax into another syntax, copying or updating
// pointers and strings as appropriate.
// NOTE: string_lists is owned by Syntax, so there's no need to
// copy it. Freeing Condition does not free any string lists.
State *merge_syntax(Syntax *syn, SyntaxMerge *merge, const ColorScheme *colors)
{
    // Generate a prefix for merged state names, to avoid clashes
    static unsigned int counter;
    char prefix[DECIMAL_STR_MAX(counter) + 2];
    xsnprintf(prefix, sizeof prefix, "m%u-", counter++);

    const HashMap *subsyn_states = &merge->subsyn->states;
    HashMap *states = &syn->states;
    char buf[FIXBUF_SIZE];

    for (HashMapIter it = hashmap_iter(subsyn_states); hashmap_next(&it); ) {
        State *s = xmemdup(it.entry->value, sizeof(State));
        s->name = xstrjoin(prefix, s->name);
        s->emit_name = xstrdup(s->emit_name);
        hashmap_insert(states, s->name, s);

        if (s->conds.count > 0) {
            // Deep copy conds PointerArray
            BUG_ON(s->conds.alloc < s->conds.count);
            void **ptrs = xnew(void*, s->conds.alloc);
            for (size_t i = 0, n = s->conds.count; i < n; i++) {
                ptrs[i] = xmemdup(s->conds.ptrs[i], sizeof(Condition));
            }
            s->conds.ptrs = ptrs;
        } else {
            BUG_ON(s->conds.alloc != 0);
        }

        // Mark unvisited so that state that is used only as a return
        // state gets visited.
        s->visited = false;

        // Don't complain about unvisited copied states.
        s->copied = true;
    }

    // Fix conditions and update colors for newly merged states
    for (HashMapIter it = hashmap_iter(subsyn_states); hashmap_next(&it); ) {
        const State *subsyn_state = it.entry->value;
        BUG_ON(!subsyn_state);
        const char *new_name = fix_name(buf, prefix, subsyn_state->name);
        State *new_state = hashmap_get(states, new_name);
        BUG_ON(!new_state);
        fix_conditions(syn, new_state, merge, prefix, buf);
        if (merge->delim) {
            update_state_colors(syn, new_state, colors);
        }
    }

    const char *name = fix_name(buf, prefix, merge->subsyn->start_state->name);
    State *start_state = hashmap_get(states, name);
    BUG_ON(!start_state);
    merge->subsyn->used = true;
    return start_state;
}
