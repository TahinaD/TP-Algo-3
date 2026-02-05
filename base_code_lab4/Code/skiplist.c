#include <stdlib.h>
#include <assert.h>

#include "skiplist.h"
#include "rng.h"

typedef struct s_Node {
	int value;
	int level;
	struct s_Node** links;
} Node;

struct s_SkipList {
	Node* sentinel;
	unsigned int size;
	RNG rng;
};

SkipList* skiplist_create(int nblevels) {
	SkipList* sl = malloc(sizeof(SkipList) + sizeof(Node) + 2 * nblevels * sizeof(Node*));
	sl->sentinel = (Node*)(sl+1);
	sl->sentinel->links = (Node**)(sl->sentinel+1);

	for (int i = 0; i < (2*nblevels); i++) {
		sl->sentinel->links[i] = sl->sentinel;
	}

	sl->size = 0;
	sl->rng = rng_initialize(0, nblevels);
	sl->sentinel->value = -1;
	sl->sentinel->level = nblevels;
	return sl;
}

void skiplist_delete(SkipList** d) {
	Node* prev_elem = (*d)->sentinel->links[(*d)->sentinel->level];
	for (Node *elem = prev_elem->links[prev_elem->level]; elem != (*d)->sentinel->links[(*d)->sentinel->level]; elem = elem->links[elem->level]) {
		free(prev_elem);
		prev_elem = elem;
	}
	free(*d);
	*d=NULL;
}

unsigned int skiplist_size(const SkipList* d) {
	return d->size;
}

int skiplist_at(const SkipList* d, unsigned int i) {
	if (i < skiplist_size(d)) {
		Node* posi = d->sentinel->links[d->sentinel->level];
		for (; i > 0; --i, posi = posi->links[posi->level]);
		return posi->value;
	}
	else
		return 0;
}

void skiplist_map(const SkipList* d, ScanOperator f, void *environment) {
	for (Node *elem = d->sentinel->links[d->sentinel->level]; elem != d->sentinel; elem = elem->links[elem->level])
		f(elem->value, environment);
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|| ./skiplisttest -c ||~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SkipList* skiplist_insert(SkipList* d, int value) {
	int cur_l = d->sentinel->level-1;
	Node** update = malloc((cur_l+1)*sizeof(Node*));
	Node* cursor = d->sentinel;
	for (; cur_l >= 0; cur_l--) {
		while (cursor->links[cursor->level + cur_l] != d->sentinel && cursor->links[cursor->level + cur_l]->value < value)
			cursor = cursor->links[cursor->level + cur_l];
		update[cur_l] = cursor;
	}

	cursor = cursor->links[cursor->level];
	if (cursor == d->sentinel || cursor->value != value) {
		int level = rng_get_value(&(d->rng))+1;
		Node* new = malloc(sizeof(Node) + 2 * level * sizeof(Node*));
		new->value = value;
		new->level = level;
		new->links = (Node**)(new+1);

		for (cur_l = 0; cur_l < level; cur_l++) {
			update[cur_l]->links[update[cur_l]->level + cur_l]->links[cur_l] = new;
			new->links[level + cur_l] = update[cur_l]->links[update[cur_l]->level + cur_l];
			update[cur_l]->links[update[cur_l]->level + cur_l] = new;
			new->links[cur_l] = update[cur_l];
		}
		(d->size)++;
	}
	free(update);
	return d;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|| ./skiplisttest -s ||~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

bool skiplist_search(const SkipList* d, int value, unsigned int *nb_operations) {
	int cur_l = d->sentinel->level-1;
	Node* cursor = d->sentinel;
	for (; cur_l >= 0; cur_l--) {
		while (cursor->links[cursor->level + cur_l] != d->sentinel && cursor->links[cursor->level + cur_l]->value < value) {
			cursor = cursor->links[cursor->level + cur_l];
			(*nb_operations)++;
		}
		if (cursor->links[cursor->level + cur_l]->value == value)
			return true;
	}
	return false;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|| ./skiplisttest -r ||~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SkipList* skiplist_remove(SkipList* d, int value) {
	int cur_l = d->sentinel->level-1;
	Node** update = malloc((cur_l+1)*sizeof(Node*));
	Node* cursor = d->sentinel;
	for (; cur_l >= 0; cur_l--) {
		while (cursor->links[cursor->level + cur_l] != d->sentinel && cursor->links[cursor->level + cur_l]->value < value)
			cursor = cursor->links[cursor->level + cur_l];
		update[cur_l] = cursor;
	}

	cursor = cursor->links[cursor->level];
	if (cursor->value == value) {
		for (cur_l = 0; cur_l < cursor->level; cur_l++) {
			update[cur_l]->links[update[cur_l]->level + cur_l] = cursor->links[cursor->level + cur_l];
			cursor->links[cursor->level + cur_l]->links[cur_l] = update[cur_l];
		}
		(d->size)--;
		free(cursor);
	}
	free(update);
	return d;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|| ./skiplisttest -i ||~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct s_SkipListIterator {
	SkipList* skiplist;
	Node* begin;
	Node* current;
	IteratorDirection direction;
};

SkipListIterator* skiplist_iterator_create(SkipList* d, IteratorDirection w) {
	SkipListIterator* it = malloc(sizeof(struct s_SkipListIterator));
	it->skiplist = d;
	it->direction = w;
	it->begin = (w == FORWARD_ITERATOR ? d->sentinel->links[d->sentinel->level] : d->sentinel->links[0]);
	it->current = it->begin;
	return it;
}

void skiplist_iterator_delete(SkipListIterator** it) {
	free(*it);
}

SkipListIterator* skiplist_iterator_begin(SkipListIterator* it) {
	it->current = it->begin;
	return it;
}

bool skiplist_iterator_end(SkipListIterator* it) {
	return it->current == it->skiplist->sentinel;
}

SkipListIterator* skiplist_iterator_next(SkipListIterator* it) {
	it->current = (it->direction == FORWARD_ITERATOR ? it->current->links[it->current->level] : it->current->links[0]);
	return it;
}

int skiplist_iterator_value(SkipListIterator* it) {
	return it->current->value;
}