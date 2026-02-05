#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"


typedef struct s_LinkedElement {
	int value;
	struct s_LinkedElement* previous;
	struct s_LinkedElement* next;
} LinkedElement;

struct s_List {
	LinkedElement* sentinel;
	int size;
};

typedef struct s_SubList {
	LinkedElement* head;
	LinkedElement* tail;
} SubList;


List* list_create(void) {
	List* l = malloc(sizeof(struct s_List) + sizeof(struct s_LinkedElement));
	l->sentinel = (LinkedElement*)(l+1);
	l->sentinel->next = l->sentinel;
	l->sentinel->previous = l->sentinel;
	l->size = 0;
	return l;
}

void list_delete(ptrList* l) {
	LinkedElement* prev_elem = (*l)->sentinel->next;
	prev_elem->previous = NULL;
	(*l)->sentinel->next = NULL;
	for (LinkedElement *elem = prev_elem->next; elem != (*l)->sentinel->next; elem = elem->next) {
		prev_elem->next = NULL;
		elem->previous = NULL;
		free(prev_elem);
		prev_elem = elem;
	}
	free(*l);
	*l=NULL;
}

List* list_push_back(List* l, int v) {
	LinkedElement* e = malloc(sizeof(LinkedElement));
	e->value = v;
	e->next = l->sentinel;
	e->previous = e->next->previous;
	e->previous->next = e;
	e->next->previous = e;
	(l->size)++;
	return l;
}

List* list_map(List* l, ListFunctor f, void* environment) {
	for (LinkedElement *elem = l->sentinel->next; elem != l->sentinel; elem = elem->next)
		elem->value = f(elem->value, environment);
	return l;
}

bool list_is_empty(const List* l) {
	return (l->size == 0);
}

int list_size(const List* l) {
	return l->size;
}

List* list_push_front(List* l, int v) {
	LinkedElement* e = malloc(sizeof(LinkedElement));
	e->value = v;
	e->previous = l->sentinel;
	e->next = e->previous->next;
	e->previous->next = e;
	e->next->previous = e;
	(l->size)++;
	return l;
}

int list_front(const List* l) {
	return l->sentinel->next->value;
}

int list_back(const List* l) {
	return l->sentinel->previous->value;
}

List* list_pop_front(List* l) {
	LinkedElement* elem = l->sentinel->next;
	l->sentinel->next = elem->next;
	l->sentinel->next->previous = l->sentinel;
	free(elem);
	(l->size)--;
	return l;
}

List* list_pop_back(List* l){
	LinkedElement* elem = l->sentinel->previous;
	l->sentinel->previous = elem->previous;
	l->sentinel->previous->next = l->sentinel;
	free(elem);
	(l->size)--;
	return l;
}

List* list_insert_at(List* l, int p, int v) {
	LinkedElement* e = malloc(sizeof(LinkedElement));
	e->value = v;
	LinkedElement* posi = l->sentinel;
	for (; p > 0; p--, posi = posi->next);
	e->previous = posi;
	e->next = posi->next;
	e->previous->next = e;
	e->next->previous = e;
	(l->size)++;
	return l;
}

List* list_remove_at(List* l, int p) {
	if (p >= 0 && p < list_size(l)) {
		LinkedElement* posi = l->sentinel->next;
		for (; p > 0; --p, posi = posi->next);
		posi->previous->next = posi->next;
		posi->next->previous = posi->previous;
		free(posi);
		(l->size)--;
	}
	return l;
}

int list_at(const List* l, int p) {
	if (p >= 0 && p < list_size(l)) {
		LinkedElement* posi = l->sentinel->next;
		for (; p > 0; --p, posi = posi->next);
		return posi->value;
	}
	else
		return 0;
}


SubList sublist_create(void) {
	SubList sl;
	sl.head = NULL;
	sl.tail = NULL;
	return sl;
}

SubList sublist_push_back(SubList sl, LinkedElement* old) {
	LinkedElement* e = malloc(sizeof(LinkedElement));
	e->value = old->value;
	e->next = NULL;
	if (sl.head == NULL) {
		sl.head = e;
		e->previous = NULL;
	}
	else {
		e->previous = sl.tail;
		sl.tail->next = e;
	}
	sl.tail = e;
	free(old);
	return sl;
}


SubList list_split(SubList l) {
	LinkedElement* elem = l.head;
	int head_l1 = 1, nb_elem = 1;
	while (elem != l.tail) {
		nb_elem++;
		elem = elem->next;
	}

	SubList split = sublist_create();
	split.head = l.head;
	for (; head_l1 < nb_elem/2 ; head_l1++)
		split.head = split.head->next;
	split.tail = split.head->next;
	return split;
}

SubList list_merge(SubList leftlist, SubList rightlist, OrderFunctor f) {
	SubList merge = sublist_create();
	LinkedElement* left_elem = (&leftlist)->head;
	LinkedElement* right_elem = (&rightlist)->head;

	while (left_elem != NULL && right_elem != NULL) {
		if (f(left_elem->value, right_elem->value)) {
			LinkedElement* left_prev = left_elem;
			left_elem = left_elem->next;
			merge = sublist_push_back(merge, left_prev);
		}
		else {
			LinkedElement* right_prev = right_elem;
			right_elem = right_elem->next;
			merge = sublist_push_back(merge, right_prev);
		}
	}

	LinkedElement* last_elem = (left_elem == NULL ? right_elem : left_elem);
	while (last_elem->next != NULL) {
		LinkedElement* last_prev = last_elem;
		last_elem = last_elem->next;
		merge = sublist_push_back(merge, last_prev);
	}
	merge = sublist_push_back(merge, last_elem);
	return merge;
}

SubList list_mergesort(SubList l, OrderFunctor f) {
	if (l.head == l.tail)
		return l;
	else {
		SubList split = list_split(l);
		SubList leftlist = sublist_create();
		SubList rightlist = sublist_create();
		leftlist.tail = split.head;
		rightlist.head = split.tail;
		leftlist.tail->next = NULL;
		rightlist.head->previous = NULL;
		leftlist.head = leftlist.tail;
		rightlist.tail = rightlist.head;

		while (leftlist.head->previous != NULL)
			leftlist.head = leftlist.head->previous;
		while (rightlist.tail->next != NULL)
			rightlist.tail = rightlist.tail->next;

		return list_merge(list_mergesort(leftlist, f), list_mergesort(rightlist, f), f);
	}
}

List* list_sort(List* l, OrderFunctor f) {
	SubList list = sublist_create();
	list.head = l->sentinel->next;
	list.tail = l->sentinel->previous;
	list.head->previous->next = NULL;
	list.tail->next->previous = NULL;
	list.head->previous = NULL;
	list.tail->next = NULL;
	list = list_mergesort(list, f);
	l->sentinel->next = list.head;
	l->sentinel->previous = list.tail;
	list.head->previous = l->sentinel;
	list.tail->next = l->sentinel;
	return l;
}

