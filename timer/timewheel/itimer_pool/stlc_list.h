/*
* Copyright (C), 2001-2010, Galaxywind Co., Ltd. 
* Description: doubly linked list header file
*
* $Id: stlc_list.h,v 1.10 2010-03-03 08:42:55 chenshijian Exp $ 
*
* $Log: not supported by cvs2svn $ 
*/

#ifndef _STLC_LIST_H
#define _STLC_LIST_H

#include <stddef.h>
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - __offsetof(type,member) );})

#define __offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define STLC_LIST_POISON1  ((void *) 0x40000000)
#define STLC_LIST_POISON2  ((void *) 0x040000004)
#define _X86_
#ifdef _X86_
static inline void prefetch_load(const void *x)
{
    return;
}
#else
#define prefetch(hint, addr) __asm__ __volatile__("pref %0, 0(%1)\n"::"i"(hint),"r"(addr))
//#ifdef FFWD
/*
#define Pref_Load			0
#define Pref_Store			1
*/
#if 0
#define prefetch(hint, addr) __asm__ __volatile__("pref %0, 0(%1)\n"::"i"(hint),"r"(addr))
#define prefetch_load(addr) prefetch((0),(addr))
#else
static inline void prefetch_load(void *addr)
{
    prefetch(0, addr);
    return;
}
#endif
//#else
//#define prefetch_load(addr) prefetch(0,addr)
//#endif
#endif
/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct stlc_list_head
{
    struct stlc_list_head *next, *prev;
};

#define STLC_LIST_HEAD_INIT(name) { &(name), &(name) }

#define STLC_LIST_HEAD(name) \
	struct stlc_list_head name = STLC_LIST_HEAD_INIT(name)

#define STLC_INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new_ex entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __stlc_list_add(struct stlc_list_head *new_ex,
                                   struct stlc_list_head *prev,
                                   struct stlc_list_head *next)
{
    next->prev = new_ex;
    new_ex->next = next;
    new_ex->prev = prev;
    prev->next = new_ex;
}

/**
 * stlc_list_add - add a new_ex entry
 * @new_ex: new_ex entry to be added
 * @head: list head to add it after
 *
 * Insert a new_ex entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void stlc_list_add(struct stlc_list_head *new_ex, struct stlc_list_head *head)
{
    __stlc_list_add(new_ex, head, head->next);
}

/**
 * stlc_list_add_tail - add a new_ex entry
 * @new_ex: new_ex entry to be added
 * @head: list head to add it before
 *
 * Insert a new_ex entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void stlc_list_add_tail(struct stlc_list_head *new_ex, struct stlc_list_head *head)
{
    __stlc_list_add(new_ex, head->prev, head);
}

/*
	add by cwh.
	Same as stlc_list_add_tail
*/
static inline void stlc_list_add_prev(struct stlc_list_head *new_ex, struct stlc_list_head *head)
{
    __stlc_list_add(new_ex, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __stlc_list_del(struct stlc_list_head *prev, struct stlc_list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * stlc_list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: stlc_list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static inline void stlc_list_del(struct stlc_list_head *entry)
{
    __stlc_list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

static inline void stlc_list_del_only(struct stlc_list_head *entry)
{
    __stlc_list_del(entry->prev, entry->next);
}


/**
 * stlc_list_replace - replace old entry by new_ex one
 * @old : the element to be replaced
 * @new_ex : the new_ex element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void stlc_list_replace(struct stlc_list_head *old,
                                     struct stlc_list_head *new_ex)
{
    new_ex->next = old->next;
    new_ex->next->prev = new_ex;
    new_ex->prev = old->prev;
    new_ex->prev->next = new_ex;
}

static inline void stlc_list_replace_init(struct stlc_list_head *old,
        struct stlc_list_head *new_ex)
{
    stlc_list_replace(old, new_ex);
    STLC_INIT_LIST_HEAD(old);
}

/**
 * stlc_list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void stlc_list_del_init(struct stlc_list_head *entry)
{
    __stlc_list_del(entry->prev, entry->next);
    STLC_INIT_LIST_HEAD(entry);
}

/**
 * stlc_list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void stlc_list_move(struct stlc_list_head *list, struct stlc_list_head *head)
{
    __stlc_list_del(list->prev, list->next);
    stlc_list_add(list, head);
}

/**
 * stlc_list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void stlc_list_move_tail(struct stlc_list_head *list,
                                       struct stlc_list_head *head)
{
    __stlc_list_del(list->prev, list->next);
    stlc_list_add_tail(list, head);
}

/**
 * stlc_list_break - break one list into two list
 * @list: the list to break
 * @entry: the element entry is the break point
 */
static inline void stlc_list_break(struct stlc_list_head *list, struct stlc_list_head *entry)
{
    struct stlc_list_head *tmp;

    tmp = entry->prev;
    tmp->next = list;
    list->prev->next = entry;
    entry->prev = list->prev;
    list->prev = tmp;
}

/**
 * stlc_list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int stlc_list_is_last(const struct stlc_list_head *list,
                                    const struct stlc_list_head *head)
{
    return list->next == head;
}

/**
 * stlc_list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int stlc_list_empty(struct stlc_list_head *head)
{
    return head->next == head;
}

static inline void __stlc_list_splice(struct stlc_list_head *list,
                                      struct stlc_list_head *head)
{
    struct stlc_list_head *first = list->next;
    struct stlc_list_head *last = list->prev;
    struct stlc_list_head *at = head->next;

    first->prev = head;
    head->next = first;

    last->next = at;
    at->prev = last;
}

/**
 * stlc_list_splice - join two lists
 * @list: the new_ex list to add.
 * @head: the place to add it in the first list.
 */
static inline void stlc_list_splice(struct stlc_list_head *list, struct stlc_list_head *head)
{
    if (!stlc_list_empty(list))
        __stlc_list_splice(list, head);
}

/**
 * stlc_list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new_ex list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void stlc_list_splice_init(struct stlc_list_head *list,
        struct stlc_list_head *head)
{
    if (!stlc_list_empty(list))
    {
        __stlc_list_splice(list, head);
        STLC_INIT_LIST_HEAD(list);
    }
}

/**
 * stlc_list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define stlc_list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * stlc_list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define stlc_list_first_entry(ptr, type, member) \
	stlc_list_entry((ptr)->next, type, member)

/**
 * stlc_list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define stlc_list_for_each(pos, head) \
	for (pos = (head)->next, prefetch_load(pos->next); pos != (head); \
        	pos = pos->next, prefetch_load(pos->next))
/**
 * stlc_list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define stlc_list_for_each_prev(pos, head) \
	for (pos = (head)->prev, prefetch_load(pos->prev); pos != (head); \
        	pos = pos->prev, prefetch_load(pos->prev))

/**
 * stlc_list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define stlc_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * stlc_list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define stlc_list_for_each_entry(pos, head, member)				\
	for (pos = stlc_list_entry((head)->next, typeof(*pos), member),	\
		     prefetch_load(pos->member.next);			\
	     &pos->member != (head); 					\
	     pos = stlc_list_entry(pos->member.next, typeof(*pos), member),	\
		     prefetch_load(pos->member.next))

/**
 * stlc_list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop counter.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define stlc_list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = stlc_list_entry((head)->next, typeof(*pos), member),	\
		n = stlc_list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = stlc_list_entry(n->member.next, typeof(*n), member))

//xqs_mod
#define stlc_list_for_each_entry_safe_prev(pos, n, head, member)			\
	    for (pos = stlc_list_entry((head)->prev, typeof(*pos), member),	\
		n = stlc_list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = stlc_list_entry(n->member.prev, typeof(*n), member))
/**
 * stlc_list_for_each_entry_continue -       iterate over list of given type
 *                      continuing after existing point
 * @pos:        the type * to use as a loop counter.
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 */
#define stlc_list_for_each_entry_continue(pos, head, member)			\
	for (pos = pos?stlc_list_entry(pos->member.next, typeof(*pos), member):stlc_list_entry((head)->next, typeof(*pos), member),	\
		     prefetch_load(pos->member.next);			\
	     &pos->member != (head);					\
	     pos = stlc_list_entry(pos->member.next, typeof(*pos), member),	\
		     prefetch_load(pos->member.next))

/**
 * stlc_list_for_each_entry_from_pos -		 iterate over list of given type
 *						from pos
 * @pos:		the type * to use as a loop counter.
 * @head:		the head for your list.
 * @member: 	the name of the list_struct within the struct.
 */
#define stlc_list_for_each_entry_from_pos(pos, head, member)			\
		for (;			\
			 &pos->member != (head);					\
			 pos = stlc_list_entry(pos->member.next, typeof(*pos), member), \
				 prefetch_load(pos->member.next))

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

struct stlc_hlist_head
{
    struct stlc_hlist_node *first;
};

struct stlc_hlist_node
{
    struct stlc_hlist_node *next, **pprev;
};

#define STLC_HLIST_HEAD_INIT { .first = NULL }
#define STLC_HLIST_HEAD(name) struct stlc_hlist_head name = {  .first = NULL }
#define STLC_INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void STLC_INIT_HLIST_NODE(struct stlc_hlist_node *h)
{
    h->next = NULL;
    h->pprev = NULL;
}

static inline int stlc_hlist_unhashed(const struct stlc_hlist_node *h)
{
    return !h->pprev;
}

/*
stlc_hlist_hashed
return none zeor if node is in hash list, else return zeor
*/
static inline int stlc_hlist_hashed(const struct stlc_hlist_node *h)
{
    if(h->pprev){
        if(h->next == STLC_LIST_POISON1 && h->pprev == STLC_LIST_POISON2)
            return 0;
        else
            return 1;
    }
    return 0;
}

static inline int stlc_hlist_empty(const struct stlc_hlist_head *h)
{
    return !h->first;
}

static inline void __stlc_hlist_del(struct stlc_hlist_node *n)
{
    struct stlc_hlist_node *next = n->next;
    struct stlc_hlist_node **pprev = n->pprev;
    *pprev = next;
    if (next)
        next->pprev = pprev;
}

static inline void stlc_hlist_del(struct stlc_hlist_node *n)
{
    __stlc_hlist_del(n);
    n->next = (struct stlc_hlist_node*)STLC_LIST_POISON1;
    n->pprev = (struct stlc_hlist_node**)STLC_LIST_POISON2;
}

static inline void stlc_hlist_del_init(struct stlc_hlist_node *n)
{
    if (!stlc_hlist_unhashed(n))
    {
        __stlc_hlist_del(n);
        STLC_INIT_HLIST_NODE(n);
    }
}

static inline void stlc_hlist_add_head(struct stlc_hlist_node *n, struct stlc_hlist_head *h)
{
    struct stlc_hlist_node *first = h->first;
    n->next = first;
    if (first)
        first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
}

/* next must be != NULL */
static inline void stlc_hlist_add_before(struct stlc_hlist_node *n,
        struct stlc_hlist_node *next)
{
    n->pprev = next->pprev;
    n->next = next;
    next->pprev = &n->next;
    *(n->pprev) = n;
}

static inline void stlc_hlist_add_after(struct stlc_hlist_node *n,
                                        struct stlc_hlist_node *next)
{
    next->next = n->next;
    n->next = next;
    next->pprev = &n->next;

    if (next->next)
        next->next->pprev  = &next->next;
}

#define stlc_hlist_entry(ptr, type, member) container_of(ptr,type,member)
#define stlc_hlist_for_each(pos, head) \
	for (pos = (head)->first; pos && ({ prefetch_load(pos->next); 1; }); \
	     pos = pos->next)

#define stlc_hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)
/**
 * stlc_hlist_for_each_entry	- iterate over list of given type
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define stlc_hlist_for_each_entry(tpos, pos, head, member)			 \
	for (pos = (head)->first;					 \
	     pos && ({ prefetch_load(pos->next); 1;}) &&			 \
		({ tpos = stlc_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * stlc_hlist_for_each_entry_continue - iterate over a hlist continuing after current point
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define stlc_hlist_for_each_entry_continue(tpos, pos, member)		 \
	for (pos = (pos)->next;						 \
	     pos && ({ prefetch_load(pos->next); 1;}) &&			 \
		({ tpos = stlc_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * stlc_hlist_for_each_entry_from - iterate over a hlist continuing from current point
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define stlc_hlist_for_each_entry_from(tpos, pos, member)			 \
	for (; pos && ({ prefetch_load(pos->next); 1;}) &&			 \
		({ tpos = stlc_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * stlc_hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @n:		another &struct hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define stlc_hlist_for_each_entry_safe(tpos, pos, n, head, member) 		 \
	for (pos = (head)->first;					 \
	     pos && ({ n = pos->next; 1; }) && 				 \
		({ tpos = stlc_hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = n)

#endif
