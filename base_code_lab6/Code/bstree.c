#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


/*------------------------  BSTreeType  -----------------------------*/

typedef enum {red, black} NodeColor;

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
    NodeColor color;
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
    t->color = red;
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

/*---------------------------  RBTSpecific  -------------------------------*/

void bstree_node_to_dot(const BinarySearchTree* t, void* stream) {
    FILE *file = (FILE *) stream;

    printf("%d ", bstree_key(t));
    if (t->color == red)
        fprintf(file, "\tn%d [style=filled, fillcolor=red, label=\"{%d|{<left>|<right>}}\"];\n",
                bstree_key(t), bstree_key(t));
    else
        fprintf(file, "\tn%d [style=filled, fillcolor=grey, label=\"{%d|{<left>|<right>}}\"];\n",
                bstree_key(t), bstree_key(t));

    if (bstree_left(t)) {
        fprintf(file, "\tn%d:left:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_left(t)));
    } else {
        fprintf(file, "\tlnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:left:c -> lnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
    if (bstree_right(t)) {
        fprintf(file, "\tn%d:right:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_right(t)));
    } else {
        fprintf(file, "\trnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:right:c -> rnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
}


void replace_child(ptrBinarySearchTree current, ptrBinarySearchTree child);

void leftrotate(BinarySearchTree *x) {
    BinarySearchTree *y = x->right;
    if (!bstree_empty(bstree_left(y)))
        y->left->parent = x;
    x->right = y->left;
    
    if (!bstree_empty(bstree_parent(x)))
        replace_child(x, y);
    
    y->parent = x->parent;
    x->parent = y;
    y->left = x;
}

void rightrotate(BinarySearchTree *y) {
    BinarySearchTree *x = y->left;
    if (!bstree_empty(bstree_right(x)))
        x->right->parent = y;
    y->left = x->right;
    
    if (!bstree_empty(bstree_parent(y)))
        replace_child(y, x);
    
    x->parent = y->parent;
    y->parent = x;
    x->right = y;
}

void testrotateleft(BinarySearchTree*t) {
    leftrotate(t);
}

void testrotateright(BinarySearchTree*t) {
    rightrotate(t);
}

BinarySearchTree* grandparent(BinarySearchTree* n) {
    assert (!bstree_empty(n) && !bstree_empty(n->parent));
    return n->parent->parent;
}

BinarySearchTree* uncle(BinarySearchTree* n) {
    assert (!bstree_empty(n) && !bstree_empty(n->parent) && !bstree_empty(n->parent->parent));
    if (n->parent->parent->left == n->parent)
        return n->parent->parent->right;
    else
        return n->parent->parent->left;
}

BinarySearchTree* brother(BinarySearchTree* p, BinarySearchTree* n) {
    assert (!bstree_empty(p));
    if (p->left == n)
        return p->right;
    else
        return p->left;
}

BinarySearchTree* fixredblack_insert_case2_left(BinarySearchTree* x) {
    BinarySearchTree* p = x->parent;
    BinarySearchTree* pp = grandparent(x);
    
    if (p->left == x){
        rightrotate(pp);
        p->color = black;
        pp->color = red;
        return p;
    } else {
        leftrotate(p);
        return fixredblack_insert_case2_left(p);
    }
}

BinarySearchTree* fixredblack_insert_case2_right(BinarySearchTree* x) {
    BinarySearchTree* p = x->parent;
    BinarySearchTree* pp = grandparent(x);

    if (p->right == x){
        leftrotate(pp);
        p->color = black;
        pp->color = red;
        return p;
    } else {
        rightrotate(p);
        return fixredblack_insert_case2_right(p);
    }
}

BinarySearchTree* fixredblack_insert_case2(BinarySearchTree* x) {
    if (x->parent->parent->left == x->parent)
        return fixredblack_insert_case2_left(x);
    else
        return fixredblack_insert_case2_right(x);
}

BinarySearchTree* fixredblack_insert(BinarySearchTree* x);

BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x) {
    if (!bstree_empty(uncle(x)) && uncle(x)->color == red) {
        x->parent->color = black;
        uncle(x)->color = black;
        grandparent(x)->color = red;
        return fixredblack_insert(grandparent(x));
    } else
        return fixredblack_insert_case2(x);
}

BinarySearchTree* fixredblack_insert(BinarySearchTree* x) {
    if (bstree_empty(bstree_parent(x)))
        x->color = black;
    else if (x->parent->color == red)
        return fixredblack_insert_case1(x);
    return x;
}


BinarySearchTree* fixredblack_remove(BinarySearchTree* p, BinarySearchTree* x);

BinarySearchTree* fixredblack_remove_case1_left(BinarySearchTree* p) {
    BinarySearchTree* f = p->right;
    BinarySearchTree* g = f->left;
    BinarySearchTree* d = f->right;

    if ((bstree_empty(g) || g->color == black) && (bstree_empty(d) || d->color == black)) {
        if (!bstree_empty(p->left))
            p->left->color = black;
        f->color = red;
        if (p->color == red) {
            p->color = black;
            return p;
        } else
            return fixredblack_remove(p->parent, p);
    } 
    
    if (d->color == red) {
        leftrotate(p);
        f->color = p->color;
        if (!bstree_empty(p->left))
            p->left->color = black;
        p->color = black;
        if (!bstree_empty(d))
            d->color = black;
        return f;
    } else {
        rightrotate(f);
        if (!bstree_empty(g))
            g->color = black;
        f->color = red;
        return fixredblack_remove_case1_left(p);
    }
}

BinarySearchTree* fixredblack_remove_case1_right(BinarySearchTree* p) {
    BinarySearchTree* f = p->left;
    BinarySearchTree* g = f->left;
    BinarySearchTree* d = f->right;

    if ((bstree_empty(g) || g->color == black) && (bstree_empty(d) || d->color == black)) {
        if (!bstree_empty(p->right))
            p->right->color = black;
        f->color = red;
        if (p->color == red) {
            p->color = black;
            return p;
        } else
            return fixredblack_remove(p->parent, p);
    } 
    
    if (g->color == red) {
        rightrotate(p);
        f->color = p->color;
        if (!bstree_empty(p->right))
            p->right->color = black;
        p->color = black;
        if (!bstree_empty(g))
            g->color = black;
        return f;
    } else {
        leftrotate(f);
        if (!bstree_empty(d))
            d->color = black;
        f->color = red;
        return fixredblack_remove_case1_right(p);
    }
}

BinarySearchTree* fixredblack_remove_case1(BinarySearchTree* p, BinarySearchTree* x) {
    if (p->left == x)
        return fixredblack_remove_case1_left(p);
    else
        return fixredblack_remove_case1_right(p);
}

BinarySearchTree* fixredblack_remove_case2_left(BinarySearchTree* p) {
    leftrotate(p);
    p->color = red;
    p->parent->color = black;
    return fixredblack_remove_case1_left(p);
}

BinarySearchTree* fixredblack_remove_case2_right(BinarySearchTree* p) {
    rightrotate(p);
    p->color = red;
    p->parent->color = black;
    return fixredblack_remove_case1_right(p);
}

BinarySearchTree* fixredblack_remove_case2(BinarySearchTree* p, BinarySearchTree* x) {
    if (p->left == x)
        return fixredblack_remove_case2_left(p);
    else
        return fixredblack_remove_case2_right(p);
}

BinarySearchTree* fixredblack_remove(BinarySearchTree* p, BinarySearchTree* x) {
    if (bstree_empty(p)) {
        if (!bstree_empty(x))
            x->color = black;
        return x;
    } else {
        if (brother(p, x)->color == black)
            return fixredblack_remove_case1(p, x);
        else
            return fixredblack_remove_case2(p, x);
    }
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

    BinarySearchTree* stop = fixredblack_insert(*cur);
    if (stop->parent == NULL)
        *t = stop;
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

void bstree_swap_pointers(ptrBinarySearchTree* from, ptrBinarySearchTree* to) {
    ptrBinarySearchTree temp = *from;
    *from = *to;
    *to = temp;
}

void replace_child(ptrBinarySearchTree current, ptrBinarySearchTree child) {
    if (!bstree_empty(bstree_parent(current))) {
        if (current->parent->left == current)
            current->parent->left = child;
        else
            current->parent->right = child;
    }
}

void bstree_swap_nodes(ptrBinarySearchTree* tree, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    if (!bstree_empty(*tree) && !bstree_empty(from) && !bstree_empty(to)){
        
        if (bstree_empty(bstree_parent(from)))
            *tree = to;
        else
            replace_child(from, to);
        
        if (bstree_empty(bstree_parent(to)))
            *tree = from;
        else
            replace_child(to, from);
        
        bstree_swap_pointers(&from->parent, &to->parent);
        
        if (!bstree_empty(bstree_left(from)))
            from->left->parent = to;
        if (!bstree_empty(bstree_left(to)))
            to->left->parent = from;

        bstree_swap_pointers(&from->left, &to->left);

        if (!bstree_empty(bstree_right(from)))
            from->right->parent = to;
        if (!bstree_empty(bstree_right(to)))
            to->right->parent = from;

        bstree_swap_pointers(&from->right, &to->right);

        NodeColor colortemp = from->color;
        from->color = to->color;
        to->color = colortemp;
    }
}

// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree* t, ptrBinarySearchTree current) {
    if (!bstree_empty(*t) && !bstree_empty(current)) {
        ptrBinarySearchTree substitute = NULL;

        if (!bstree_empty(bstree_left(current)) && !bstree_empty(bstree_right(current))){
            bstree_swap_nodes(t, current, (ptrBinarySearchTree) bstree_successor(current));
            substitute = current->right;
        } else {  
            if (!bstree_empty(bstree_left(current)))
                substitute = current->left;
            
            if (!bstree_empty(bstree_right(current)))
                substitute = current->right;
        }

        if (!bstree_empty(substitute))
            substitute->parent = current->parent;
            
        if (!bstree_empty(bstree_parent(current)))
            replace_child(current, substitute);
        else
            *t = substitute;

        if (current->color == black) {
            if ((substitute == NULL) || (substitute->color == black)) {
                ptrBinarySearchTree subtreeroot = fixredblack_remove(current->parent, substitute);
                if (subtreeroot->parent == NULL)
                    *t = subtreeroot;
            } else
                substitute->color = black;
        }
        
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
    while (e->left) {
        e = e->left;
    }
	return e;
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
    while (e->right) {
        e = e->right;
    }
	return e;
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
	BSTreeIterator* i = malloc(sizeof(struct _BSTreeIterator));
    i->collection = collection;
    if (direction == forward) {
        i->begin = goto_min;
        i->next = bstree_successor;
    } else {
        i->begin = goto_max;
        i->next = bstree_predecessor;
    }
    i->current = i->begin(collection);
	return i;
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


