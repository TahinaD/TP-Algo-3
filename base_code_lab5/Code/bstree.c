#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


/*------------------------  BSTreeType  -----------------------------*/

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
};

/*------------------------  BaseBSTree  -----------------------------*/

BinarySearchTree* bstree_create(void) {
    return NULL;
}

/* This constructor is private so that we can maintain the oredring invariant on
 * nodes. The only way to add nodes to the tree is with the bstree_add function
 * that ensures the invariant.
 */
BinarySearchTree* bstree_cons(BinarySearchTree* left, BinarySearchTree* right, int key) {
    BinarySearchTree* t = malloc(sizeof(struct _bstree));
    t->parent = NULL;
    t->left = left;
    t->right = right;
    if (t->left != NULL)
        t->left->parent = t;
    if (t->right != NULL)
        t->right->parent = t;
    t->key = key;
    return t;
}

void freenode(const BinarySearchTree* t, void* n) {
    (void)n;
    free((BinarySearchTree*)t);
}

void bstree_delete(ptrBinarySearchTree* t) {
    bstree_depth_postfix(*t, freenode, NULL);
    *t=NULL;
}

bool bstree_empty(const BinarySearchTree* t) {
    return t == NULL;
}

int bstree_key(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->key;
}

BinarySearchTree* bstree_left(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->left;
}

BinarySearchTree* bstree_right(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->right;
}

BinarySearchTree* bstree_parent(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->parent;
}

/*------------------------  BSTreeDictionary  -----------------------------*/

/* Obligation de passer l'arbre par référence pour pouvoir le modifier */
void bstree_add(ptrBinarySearchTree* t, int v) {
    ptrBinarySearchTree* cur = t;
    BinarySearchTree* par = NULL;
    while (*cur) {
        if (v == bstree_key(*cur))
            return;

        par = *cur;
        if (v < bstree_key(*cur))
            cur = &((*cur)->left);
        else
            cur = &((*cur)->right);
    }
    *cur = bstree_cons(NULL, NULL, v);
    (*cur)->parent = par;
}

const BinarySearchTree* bstree_search(const BinarySearchTree* t, int v) {
    const BinarySearchTree* cur = t;
    while (cur) {
        if (v == bstree_key(cur))
            return cur;
        else if (v < bstree_key(cur))
            cur = bstree_left(cur);
        else
            cur = bstree_right(cur);
    }
    return NULL;
}

typedef BinarySearchTree* (*AccessFunction) (const BinarySearchTree* t);

typedef struct s_ChildAccessors {
    AccessFunction prev;
    AccessFunction next;
} ChildAccessors;

BinarySearchTree* find_next(const BinarySearchTree* x, ChildAccessors access) {
    BinarySearchTree* cur = (BinarySearchTree*) x;
    if (!bstree_empty(access.next(cur))){
        cur = access.next(cur);
        while (!bstree_empty(access.prev(cur)))
            cur = access.prev(cur);
        return cur;
    }

    else {
        while (!bstree_empty(bstree_parent(cur)) && cur != access.prev(bstree_parent(cur)))
            cur = bstree_parent(cur);
        return bstree_parent(cur);
    }
}

const BinarySearchTree* bstree_successor(const BinarySearchTree* x) {
    ChildAccessors children;
    children.prev = bstree_left;
    children.next = bstree_right;
    return find_next(x, children);
}

const BinarySearchTree* bstree_predecessor(const BinarySearchTree* x) {
    ChildAccessors children;
    children.prev = bstree_right;
    children.next = bstree_left;
    return find_next(x, children);
}

void bstree_swap_pointers(ptrBinarySearchTree from, ptrBinarySearchTree to) {
    BinarySearchTree temp = *from;
    *from = *to;
    *to = temp;
}

void replace_child(ptrBinarySearchTree current, ptrBinarySearchTree child) {
    if (!bstree_empty(bstree_parent(current))) {
        if (bstree_left(bstree_parent(current)) == current)
            current->parent->left = child;
        else
            current->parent->right = child;
    }
}

void bstree_swap_nodes(ptrBinarySearchTree* tree, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    
    bstree_swap_pointers(from, to);
    printf("current : %d and successor : %d \n", from->key, to->key);
    
    if (!bstree_empty(bstree_parent(from)))
        *tree = from;
    else
        replace_child(to, from);
    
    if (!bstree_empty(bstree_parent(to)))
        *tree = to;
    else
        replace_child(from, to);
    

    printf("swap parent ok \n");
    if (!bstree_empty(bstree_left(from)))
        from->left->parent = to;
    if (!bstree_empty(bstree_left(from)))
        from->left->parent = to;

    printf("swap left ok \n");
    if (!bstree_empty(bstree_right(to)))
        to->right->parent = from;
    if (!bstree_empty(bstree_right(to)))
        to->right->parent = from;
    printf("swap right ok \n");
    
    printf("from : %d and to : %d \n", from->parent->key, to->parent->key);
    bstree_swap_pointers(from->parent, to->parent);
    bstree_swap_pointers(from->left, to->left);
    bstree_swap_pointers(from->right, to->right);
    
    
}

// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree* t, ptrBinarySearchTree current) {
    if (!bstree_empty(*t) && !bstree_empty(current)) {
        if (!bstree_empty(bstree_left(current)) && bstree_empty(bstree_right(current))) {
            current->left->parent = current->parent;
            replace_child(current, current->left);
        }
        else if (!bstree_empty(bstree_right(current)) && bstree_empty(bstree_left(current))){
            current->right->parent = current->parent;
            replace_child(current, current->right);
        }
        else if (!bstree_empty(bstree_left(current)) && !bstree_empty(bstree_right(current)))
            bstree_swap_nodes(t, current, (ptrBinarySearchTree) bstree_successor(current));
        free(current);
    }
}

void bstree_remove(ptrBinarySearchTree* t, int v) {
    ptrBinarySearchTree node = (ptrBinarySearchTree) bstree_search(*t, v);
    bstree_remove_node(t, node);
}

/*------------------------  BSTreeVisitors  -----------------------------*/

void bstree_depth_prefix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (!bstree_empty(t)){
        f(t, environment);
        bstree_depth_prefix(bstree_left(t), f, environment);
        bstree_depth_prefix(bstree_right(t), f, environment);
    }
}

void bstree_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (!bstree_empty(t)){
        bstree_depth_infix(bstree_left(t), f, environment);
        f(t, environment);
        bstree_depth_infix(bstree_right(t), f, environment);
    }
}

void bstree_depth_postfix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (!bstree_empty(t)){
        bstree_depth_postfix(bstree_left(t), f, environment);
        bstree_depth_postfix(bstree_right(t), f, environment);
        f(t, environment);
    }
}

void bstree_iterative_breadth(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    Queue* q = create_queue();
    if (!bstree_empty(t))
        queue_push(q, t);
    
    while (!queue_empty(q)) {
        BinarySearchTree* cur = (BinarySearchTree*) queue_top(q);
        queue_pop(q);
        f(cur, environment);
        if (!bstree_empty(bstree_left(cur)))
            queue_push(q, bstree_left(cur));
        if (!bstree_empty(bstree_right(cur)))
            queue_push(q, bstree_right(cur));
    }
    delete_queue(&q);
}

void bstree_iterative_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    BinarySearchTree* current = (BinarySearchTree*) t;
    BinarySearchTree* prev = current->parent;
    BinarySearchTree* next = current->parent;

    while (!bstree_empty(current)) {
        if (prev == bstree_parent(current)) {
            prev = current;
            next = current->left;
        }
        if (bstree_empty(next) || prev == bstree_left(current)) {
            f(current, environment);
            prev = current;
            next = current->right;
        }
        if (bstree_empty(next) || prev == bstree_right(current)) {
            prev = current;
            next = current->parent;
        }
        current = next;
    }
}

/*------------------------  BSTreeIterator  -----------------------------*/

struct _BSTreeIterator {
    /* the collection the iterator is attached to */
    const BinarySearchTree* collection;
    /* the first element according to the iterator direction */
    const BinarySearchTree* (*begin)(const BinarySearchTree* );
    /* the current element pointed by the iterator */
    const BinarySearchTree* current;
    /* function that goes to the next element according to the iterator direction */
    const BinarySearchTree* (*next)(const BinarySearchTree* );
};

/* minimum element of the collection */
const BinarySearchTree* goto_min(const BinarySearchTree* e) {
	(void)e;
	return NULL;
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
	(void)e;
	return NULL;
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
	(void)collection; (void)direction;
	return NULL;
}

/* destructor */
void bstree_iterator_delete(ptrBSTreeIterator* i) {
    free(*i);
    *i = NULL;
}

BSTreeIterator* bstree_iterator_begin(BSTreeIterator* i) {
    i->current = i->begin(i->collection);
    return i;
}

bool bstree_iterator_end(const BSTreeIterator* i) {
    return i->current == NULL;
}

BSTreeIterator* bstree_iterator_next(BSTreeIterator* i) {
    i->current = i->next(i->current);
    return i;
}

const BinarySearchTree* bstree_iterator_value(const BSTreeIterator* i) {
    return i->current;
}

