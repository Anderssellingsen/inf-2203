/**
 * @file
 * List implementation <list.h>
 *
 * Circular doubly-linked list implementation.
 *
 * Adapted from Linux's linked list implementation (include/linux/list.h).
 *
 * @see
 *
 * - Linux kernel documentation: <https://docs.kernel.org/core-api/list.html>
 * - Linux kernel source code: include/linux/list.h
 */
#ifndef LIST_H
#define LIST_H

#include <core/macros.h>

#include <stddef.h>

/** @name List construction. */
///@{

/**
 * Circular doubl-linked list pointers.
 *
 *      .-> head  --> item0 --,
 *      `-- item2 <-- item1 <-'
 */
struct list_head {
    struct list_head *next, *prev;
};

/**
 * Static initializer for a list_head
 *
 * Set both next and prev to point to itself, so that the @ref list_head is
 * circularly linked to itself.
 *
 * @param   name    Name of list_head instance variable.
 *                  This is needed so that the list head can be initialized
 *                  with pointers to itself.
 */
#define LIST_HEAD_INIT(name) \
    { \
        .next = &(name), .prev = &(name) \
    }

/**
 * Define and statically initialize a list_head
 *
 * @param   name    Name of list_head instance variable.
 */
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

/**
 * Run-time initialization for a list_head
 *
 * @param   list    Pointer to list to initialize.
 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void _list_ensure_init(struct list_head *list)
{
    if (!list->next) INIT_LIST_HEAD(list);
}

///@}

/** @name List checks */
///@{

/**
 * Checks if the list is empty.
 */
static inline int list_empty(struct list_head *head)
{
    return !head->next || head->next == head;
}

/**
 * Tests whether the given list node is the list head
 */
static inline int
list_is_head(const struct list_head *list, const struct list_head *head)
{
    return list == head;
}

/**
 * Tests whether the given list node is first in the list
 */
static inline int
list_is_first(const struct list_head *list, const struct list_head *head)
{
    return list->prev == head;
}

/**
 * Tests whether the given list node is last in the list
 */
static inline int
list_is_last(const struct list_head *list, const struct list_head *head)
{
    return list->next == head;
}

///@}

/** @name List add/remove */
///@{

/**
 * Inner list add function.
 *
 * Adds `new` item between `a` and `b`.
 *
 *      .-> head  --> a --,
 *      `--  b  <-- new <-'
 */
static inline void
_list_add(struct list_head *new, struct list_head *a, struct list_head *b)
{
    a->next   = new;
    new->prev = a;
    new->next = b;
    b->prev   = new;
}

/**
 * Add item to front of list (after head).
 *
 * Adds the `new` entry after `head`, which will make it next in the list.
 * If `head` is the head of the list, `new` will be at the front.
 *
 *      .-> head  --> new   --,
 *      `-- item2 <-- item1 <-'
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
    _list_ensure_init(head);
    _list_add(new, head, head->next);
}

/**
 * Add item to back of list (before head).
 *
 * Adds the `new` entry before `head`, which will wrap around and put it last.
 *
 *      .-> head --> item0 --,
 *      `-- new  <-- item1 <-'
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    _list_ensure_init(head);
    _list_add(new, head->prev, head);
}

/**
 * Remove item from list.
 *
 * Takes `item` out of the list by stitching together its previous and next
 * items.
 *
 *                      item
 *      .-> head  ------------,
 *      `-- item2 <-- item1 <-'
 */
static inline void list_del(struct list_head *item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;

    item->next = NULL;
    item->prev = NULL;
}

///@}

/** @name List entry retrieval */
///@{

/**
 * Get a pointer to the entry struct from a pointer to its list_head field.
 *
 * This macro takes a pointer to a @ref list_head and returns a pointer to
 * the containing entry struct. This is useful after traversing the list
 * to find an item.
 *
        list_entry(ptr) --> +-------+   +-------+   +-------+
                            | item  |   | item  |   | item  |
                            +-------+   +-------+   +-------+
                   ptr ---> | p | n |<->| p | n |<->| p | n |
                            +-------+   +-------+   +-------+
 *
 * @param   ptr     Pointer to the list_head field within a struct.
 * @param   type    The type of the item struct.
 * @param   member  The name of the list_head field within that struct type.
 * @returns
 *      A pointer to the item struct.
 *
 * @note
 *     Be careful to never call this on the free-floating head of the list.
 *     It does not have a corresponding entry to get, and attempting to
 *     do so will cause @ref undefined_behavior.
 *
            list_entry(head) --> ????       +-------+   +-------+
                                            | item  |   | item  |
                                +-------+   +-------+   +-------+
                       head --> | p | n |<->| p | n |<->| p | n |
                                +-------+   +-------+   +-------+
 *
 * This macro delgates to the @ref container_of macro. See its documentation
 * for details.
 */
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/**
 * Get a pointer to the first entry in a list
 *
 * @param   ptr     Pointer to list_head.
 * @param   type    The type of the item struct.
 * @param   member  The name of the list_head field within that struct type.
 * @returns
 *      A pointer to the item struct for the first item in the list.
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

/**
 * Get a pointer to the last entry in a list
 *
 * @param   ptr     Pointer to list_head.
 * @param   type    The type of the item struct.
 * @param   member  The name of the list_head field within that struct type.
 * @returns
 *      A pointer to the item struct for the last item in the list.
 */
#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

/**
 * Remove and return the first item in the list.
 *
 *                      item
 *      .-> head  ------------,
 *      `-- item2 <-- item1 <-'
 *
 * @param   head    The list to take from.
 * @returns The next @ref list_head in list, or NULL if the list was empty.
 */
static inline struct list_head *list_shift(struct list_head *head)
{
    if (list_empty(head)) return NULL;
    struct list_head *first = head->next;
    list_del(first);
    return first;
}

/**
 * Remove and return the first entry in the list.
 *
 * This removes the next item in the list, and then uses @ref list_entry
 * to return a pointer to the item itself rather than the @ref list_head field.
 *
 *                      item
 *      .-> head  ------------,
 *      `-- item2 <-- item1 <-'
 *
 * @param   head    The list to take from.
 * @param   type    The entry type of this list.
 * @param   member  The name of the @ref list_head field within the _type_.
 *
 * @returns A pointer to the _type_ struct of the next entry in the list.
 */
#define list_shift_entry(head, type, member) \
    (list_entry(list_shift(head), type, member))

/**
 * Test if an entry cursor points to the head of the list
 *
 * If it does point to the head of the list, the dereferencing it is
 * @ref undefined_behavior.
 *
 * @param   pos     Cursor pointer, a pointer of the list's entry type.
 * @param   head    Pointer to list_head for list.
 * @param   member  The name of the list_head field within entry struct type.
 */
#define list_entry_is_head(pos, head, member) \
    list_is_head(&(pos)->member, (head))

/**
 * Get the next entry in the list, after cursor
 *
 * @param   pos     Cursor pointer, a pointer of the list's entry type.
 * @param   member  The name of the list_head field within entry struct type.
 * @returns
 *      A pointer to the entry struct for the next item after pos.
 */
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * Get the previous entry in the list, before cursor
 *
 * @param   pos     Cursor pointer, a pointer of the list's entry type.
 * @param   member  The name of the list_head field within entry struct type.
 * @returns
 *      A pointer to the entry struct for the previous item before pos.
 */
#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

///@}

/** @name List rotation */
///@{

/**
 * Rotate the list left (to next).
 *
 * Rotate the `head` along its next pointer.
 *
 *      .-> head  --> item1 --,
 *      `-- item0 <-- item2 <-'
 */
static inline void list_rotate_left(struct list_head *head)
{
    if (list_empty(head)) return;
    struct list_head *first = head->next;
    list_del(head);        // Remove the head from its own list.
    list_add(head, first); // Re-insert the head after the first item.
}

///@}

/** @name List iteration */
///@{

/**
 * Expands into a `for` statement that iterates over a list
 *
 * Iteration starts at the first item in the list (head->next) and visits
 * each list_head item in the list.
 *
 * @param   pos     Cursor pointer, a pointer of type struct list_head.
 *                  Must be declared elsewhere.
 * @param   head    Pointer to list head.
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)

/**
 * Expands into a `for` statement that iterates over a list's entries
 *
 * Iteration starts at the first entry in the list (@ref list_first_entry) and
 * visits each entry in the list.
 *
 * @param   pos     Cursor pointer, a pointer of the list's entry type.
 *                  Must be declared elsewhere.
 * @param   head    Pointer to list head.
 * @param   member  The name of the list_head field within entry struct type.
 */
#define list_for_each_entry(pos, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member); \
         !list_entry_is_head(pos, head, member); \
         pos = list_next_entry(pos, member))

/**
 * Expands into a `for` statement that iterates a list's entries in reverse
 *
 * Iteration starts at the last entry in the list (@ref list_last_entry) and
 * visits each entry in the list.
 *
 * @param   pos     Cursor pointer, a pointer of the list's entry type.
 *                  Must be declared elsewhere.
 * @param   head    Pointer to list head.
 * @param   member  The name of the list_head field within entry struct type.
 */
#define list_for_each_entry_prev(pos, head, member) \
    for (pos = list_last_entry(head, typeof(*pos), member); \
         !list_entry_is_head(pos, head, member); \
         pos = list_prev_entry(pos, member))

/**
 * Iteration over a list that tolerates removal of the current item
 *
 * @param   pos     Cursor pointer, a pointer of type struct list_head.
 *                  Must be declared elsewhere.
 * @param   n       A second list_head pointer for temporay storage.
 * @param   head    Pointer to list head.
 */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; !list_is_head(pos, (head)); \
         pos = n; n = pos->next)

/**
 * Iteration over a list's entries that tolerates removal of the current entry
 *
 * @param   pos     Cursor pointer, a pointer of the list's entry type.
 *                  Must be declared elsewhere.
 * @param   n       A second entry type pointer for temporay storage.
 * @param   head    Pointer to list head.
 * @param   member  The name of the list_head field within entry struct type.
 */
#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member), \
        n    = list_next_entry(pos, member); \
         !list_entry_is_head(pos, head, member); \
         pos = n, n = list_next_entry(n, member))

///@}
#endif /* LIST_H */
