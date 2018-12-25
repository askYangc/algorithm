#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "rbtree.h"



struct memfs_file {
    char *path;                     /* File path */
    void *data;                     /* File content */
    int free_on_delete;
 
    struct stat vstat;              /* File stat */
 
    pthread_mutex_t lock;
    pthread_cond_t write_finished;
 
    struct rb_node node;
};

static int __insert(struct rb_root *root, struct memfs_file *pf)
{
    struct rb_node **new = &(root->rb_node), *parent = NULL;
 
    /* Figure out where to put new node */
    while (*new) {
        struct memfs_file *this = container_of(*new, struct memfs_file, node);
        int result = strcmp(pf->path, this->path);
        parent = *new;
        if (result < 0) {
            new = &((*new)->rb_left);
        }
        else if (result > 0) {
            new = &((*new)->rb_right);
        }
        else {
            printf("%s is exist!!!\n", pf->path);
            return -1;
        }
    }
 
    /* Add new node and rebalance tree. */
    rb_link_node(&pf->node, parent, new);
    rb_insert_color(&pf->node, root);
    return 0;

}


static struct memfs_file *__search(struct rb_root *root, const char *path)
{
    struct memfs_file *pf = NULL;
    struct rb_node *node = root->rb_node;
 
    while (node) {
        pf = container_of(node, struct memfs_file, node);
 
        int result = strcmp(path, pf->path);
        if (result < 0) {
            node = node->rb_left;
        }
        else if (result > 0) {
            node = node->rb_right;
        }
        else {
            return pf;
        }
    }
 
//  printf("%s not found!!!\n", path);
    return NULL;
}

static inline void __free(struct memfs_file *pf)
{
    if (pf->free_on_delete) {
        if (pf->data) {
            free(pf->data);
        }
        if (pf->path) {
            free(pf->path);
        }
        free(pf);
    }
}
 
static int __delete(struct rb_root *root, const char *path)
{
    struct memfs_file *pf = __search(root, path);
    if (!pf) {
        return -1;
    }   
    
    int blocks = pf->vstat.st_blocks;
    rb_erase(&pf->node, root);
    __free(pf);
 
    return blocks;
}

int main(int argc, char *argv[])
{
    struct rb_root root = RB_ROOT;
 
    struct memfs_file files[] = {
        [0] = { "/tmp/a.jpg",       "0x1234", },
        [1] = { "/tmp/c.txt",       "0x1234", },
        [2] = { "/tmp/d.exe",       "0x1234", },
        [3] = { "/tmp/b.dir/b.txt", "0x1234", },
    };
 
    __insert(&root, &files[0]);
    __insert(&root, &files[1]);
    __insert(&root, &files[2]);
    __insert(&root, &files[3]);
 
 
#define TEST_SEARCH(root, name) do { \
    const struct memfs_file *pf = __search(&root, name); \
    assert(strcmp(pf->path, name) == 0); \
    printf("search: %s\t\t\t\t[OK]\n", name); \
} while(0)
 
    TEST_SEARCH(root, files[0].path);
    TEST_SEARCH(root, files[1].path);
    TEST_SEARCH(root, files[2].path);
    TEST_SEARCH(root, files[3].path);
 
    /* Trasver all */
    if (1) {
        struct rb_node *node = NULL;
        for (node = rb_first(&root); node; node = rb_next(node)) {
            const struct memfs_file *pf = rb_entry(node, struct memfs_file, node);
            printf("iterator: %s\t\t\t\t[OK]\n", pf->path);
        }
    }
 
#define TEST_DELETE(root, name) do { \
    __delete(&root, name); \
    assert(__search(&root, name) == NULL); \
    printf("delete: %s\t\t\t\t[OK]\n", name); \
} while(0)
 
    TEST_DELETE(root, files[0].path);
    exit(EXIT_SUCCESS);
}


