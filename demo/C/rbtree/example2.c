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

int memfs_file_cmp(struct memfs_file *cur, struct memfs_file *pf)
{
	return strcmp(pf->path, cur->path);
}
	
	
int memfs_file_search(struct memfs_file *cur, const char *path)
{
	return strcmp(path, cur->path);
}


static inline void __free(struct memfs_file *pf)
{
	if(pf == NULL) return;
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

int main(int argc, char *argv[])
{
    struct rb_root root = RB_ROOT;
 
    struct memfs_file files[] = {
        [0] = { "/tmp/a.jpg",       "0x1234", },
        [1] = { "/tmp/c.txt",       "0x1234", },
        [2] = { "/tmp/d.exe",       "0x1234", },
        [3] = { "/tmp/b.dir/b.txt", "0x1234", },
		[4] = { "/tmp/ddd.dir/b.txt", "0x1234", },
    };


	rb_insert(&root, &files[0], struct memfs_file, node, memfs_file_cmp);
	rb_insert(&root, &files[1], struct memfs_file, node, memfs_file_cmp);
	rb_insert(&root, &files[2], struct memfs_file, node, memfs_file_cmp);
	rb_insert(&root, &files[3], struct memfs_file, node, memfs_file_cmp);
	rb_insert(&root, &files[0], struct memfs_file, node, memfs_file_cmp);

 
 
#define TEST_SEARCH1(root, name) do { \
    const struct memfs_file *pf = __search(root, name); \
    assert(strcmp(pf->path, name) == 0); \
    printf("search: %s\t\t\t\t[OK]\n", name); \
} while(0)

#define TEST_SEARCH(root, name) do { \
		struct memfs_file *pos;\
		rb_search(root, pos, struct memfs_file, node, memfs_file_search, name);\
		if(pos){\
			assert(strcmp(pos->path, name) == 0); \
			printf("search: %s\t\t\t\t[OK]\n", name); \
		}else {\
			printf("search: %s\t\t\t\t[not found]\n", name); \
		}\
	} while(0)

 
    TEST_SEARCH(&root, files[0].path);
    TEST_SEARCH(&root, files[1].path);
    TEST_SEARCH(&root, files[2].path);
    TEST_SEARCH(&root, files[4].path);
 
    /* Trasver all */
    if (1) {
        struct rb_node *node = NULL;
        for (node = rb_first(&root); node; node = rb_next(node)) {
            const struct memfs_file *pf = rb_entry(node, struct memfs_file, node);
            printf("iterator: %s\t\t\t\t[OK]\n", pf->path);
        }
    }

//this maybee Memory leaks	
#define TEST_DELETE(root, name) do { \
    rb_delete(root, struct memfs_file, node, memfs_file_search, name);\
    TEST_SEARCH(root, name);\
    printf("delete: %s\t\t\t\t[OK]\n", name); \
} while(0)

#define TEST_DELETE1(root, name) do { \
		struct memfs_file *pos;\
		rb_search(root, pos, struct memfs_file, node, memfs_file_search, name);\
		rb_delete_node(root, pos, node);\
		__free(pos);\
		TEST_SEARCH(root, name);\
		printf("delete: %s\t\t\t\t[OK]\n", name); \
	} while(0)

 
    TEST_DELETE1(&root, files[4].path);

	printf("-----------\n");
    /* Trasver all */
    if (1) {
        struct rb_node *node = NULL;
        for (node = rb_first(&root); node; node = rb_next(node)) {
            const struct memfs_file *pf = rb_entry(node, struct memfs_file, node);
            printf("iterator: %s\t\t\t\t[OK]\n", pf->path);
        }
    }

	
	if(1) {
		struct rb_node *node = NULL;
		for (node = rb_first(&root); node; node = rb_next(node)) {
			struct memfs_file *pf = rb_entry(node, struct memfs_file, node);
			rb_delete_node(&root, pf, node);
			__free(pf);
		}

	}


	printf("------find node-----\n");
    /* Trasver all */
    if (1) {
        struct rb_node *node = NULL;
        for (node = rb_first(&root); node; node = rb_next(node)) {
            const struct memfs_file *pf = rb_entry(node, struct memfs_file, node);
            printf("iterator: %s\t\t\t\t[OK]\n", pf->path);
        }
    }

	if(root.rb_node == NULL) {
		printf("rb_node is NULL\n");
	}

    exit(EXIT_SUCCESS);
}


