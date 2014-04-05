/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _AO_LIST_H_
#define _AO_LIST_H_

#include <stddef.h>

struct ao_list {
	struct ao_list	*next, *prev;
};

static inline void
ao_list_init(struct ao_list *list)
{
	list->next = list->prev = list;
}

static inline void
__ao_list_add(struct ao_list *list, struct ao_list *prev, struct ao_list *next)
{
	next->prev = list;
	list->next = next;
	list->prev = prev;
	prev->next = list;
}

/**
 * Insert a new element after the given list head. The new element does not
 * need to be initialised as empty list.
 * The list changes from:
 *      head → some element → ...
 * to
 *      head → new element → older element → ...
 *
 * Example:
 * struct foo *newfoo = malloc(...);
 * ao_list_add(&newfoo->entry, &bar->list_of_foos);
 *
 * @param entry The new element to prepend to the list.
 * @param head The existing list.
 */
static inline void
ao_list_insert(struct ao_list *entry, struct ao_list *head)
{
    __ao_list_add(entry, head, head->next);
}

/**
 * Append a new element to the end of the list given with this list head.
 *
 * The list changes from:
 *      head → some element → ... → lastelement
 * to
 *      head → some element → ... → lastelement → new element
 *
 * Example:
 * struct foo *newfoo = malloc(...);
 * ao_list_append(&newfoo->entry, &bar->list_of_foos);
 *
 * @param entry The new element to prepend to the list.
 * @param head The existing list.
 */
static inline void
ao_list_append(struct ao_list *entry, struct ao_list *head)
{
    __ao_list_add(entry, head->prev, head);
}

static inline void
__ao_list_del(struct ao_list *prev, struct ao_list *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * Remove the element from the list it is in. Using this function will reset
 * the pointers to/from this element so it is removed from the list. It does
 * NOT free the element itself or manipulate it otherwise.
 *
 * Using ao_list_del on a pure list head (like in the example at the top of
 * this file) will NOT remove the first element from
 * the list but rather reset the list as empty list.
 *
 * Example:
 * ao_list_del(&foo->entry);
 *
 * @param entry The element to remove.
 */
static inline void
ao_list_del(struct ao_list *entry)
{
    __ao_list_del(entry->prev, entry->next);
    ao_list_init(entry);
}

/**
 * Check if the list is empty.
 *
 * Example:
 * ao_list_is_empty(&bar->list_of_foos);
 *
 * @return True if the list contains one or more elements or False otherwise.
 */
static inline uint8_t
ao_list_is_empty(struct ao_list *head)
{
    return head->next == head;
}

/**
 * Returns a pointer to the container of this list element.
 *
 * Example:
 * struct foo* f;
 * f = container_of(&foo->entry, struct foo, entry);
 * assert(f == foo);
 *
 * @param ptr Pointer to the struct ao_list.
 * @param type Data type of the list element.
 * @param member Member name of the struct ao_list field in the list element.
 * @return A pointer to the data struct containing the list head.
 */
#define ao_container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

/**
 * Alias of ao_container_of
 */
#define ao_list_entry(ptr, type, member) \
    ao_container_of(ptr, type, member)

/**
 * Retrieve the first list entry for the given list pointer.
 *
 * Example:
 * struct foo *first;
 * first = ao_list_first_entry(&bar->list_of_foos, struct foo, list_of_foos);
 *
 * @param ptr The list head
 * @param type Data type of the list element to retrieve
 * @param member Member name of the struct ao_list field in the list element.
 * @return A pointer to the first list element.
 */
#define ao_list_first_entry(ptr, type, member) \
    ao_list_entry((ptr)->next, type, member)

/**
 * Retrieve the last list entry for the given listpointer.
 *
 * Example:
 * struct foo *first;
 * first = ao_list_last_entry(&bar->list_of_foos, struct foo, list_of_foos);
 *
 * @param ptr The list head
 * @param type Data type of the list element to retrieve
 * @param member Member name of the struct ao_list field in the list element.
 * @return A pointer to the last list element.
 */
#define ao_list_last_entry(ptr, type, member) \
    ao_list_entry((ptr)->prev, type, member)

/**
 * Loop through the list given by head and set pos to struct in the list.
 *
 * Example:
 * struct foo *iterator;
 * ao_list_for_each_entry(iterator, &bar->list_of_foos, entry) {
 *      [modify iterator]
 * }
 *
 * This macro is not safe for node deletion. Use ao_list_for_each_entry_safe
 * instead.
 *
 * @param pos Iterator variable of the type of the list elements.
 * @param head List head
 * @param member Member name of the struct ao_list in the list elements.
 *
 */
#define ao_list_for_each_entry(pos, head, type, member)			\
    for (pos = ao_container_of((head)->next, type, member);		\
	 &pos->member != (head);					\
	 pos = ao_container_of(pos->member.next, type, member))

/**
 * Loop through the list, keeping a backup pointer to the element. This
 * macro allows for the deletion of a list element while looping through the
 * list.
 *
 * See ao_list_for_each_entry for more details.
 */
#define ao_list_for_each_entry_safe(pos, tmp, head, type, member)		\
	for ((pos = ao_container_of((head)->next, type, member)),		\
	     (tmp = ao_container_of(pos->member.next, type, member)); 		\
	     &pos->member != (head);						\
	     (pos = tmp), (tmp = ao_container_of(pos->member.next, type, member)))

#endif /* _AO_LIST_H_ */
