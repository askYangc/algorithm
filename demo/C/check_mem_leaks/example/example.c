#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *test(void *p)
{
    int i = 0;
    int args = *(int*)p;

    for(i = 1; i <= 100;i++) {
        printf("now %d malloc %d\n", args, i);
#if 1
        char *p = malloc(i);
        char *d = calloc(1, i);
        char *f = realloc(d, 15);
        free(p);
        free(f);
#endif
    }
    return NULL;
}


int main1()
{
    int i = 0;
    pthread_t pid[4];
    int a[4] = {1,2,3,4};

    for(i = 0; i < 4; i++) {
        pthread_create(&pid[i], NULL, test, &a[i]);
    }
    
    for(i = 0; i < 4; i++) {
        pthread_join(pid[i], NULL);
    }
    

    return 0;
}

void test4()
{
    void *p = malloc(211);
    p = malloc(1032);
    p = malloc(160);
    p = malloc(1592);
    mem_leaks_show();
    free(p);
    
    printf("okok\n");
}



void test3()
{
    test4();
}


void test2()
{
    test3();
}


void test1()
{
    test2();
}


int main()
{
    mem_leaks_start(); 
    test1();
    mem_leaks_show();

    return 0;
}


