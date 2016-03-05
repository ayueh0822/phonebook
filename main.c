#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include IMPL

#define DICT_FILE "./dictionary/words.txt"
static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int main(int argc, char *argv[])
{
    FILE *fp;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];
    struct timespec start, end;
    double cpu_time1, cpu_time2;

    /* check file opening */
    fp = fopen(DICT_FILE, "r");
    if (fp == NULL) {
        printf("cannot open the file\n");
        return -1;
    }

    /* build the entry */
    entry *pHead, *e;
    pHead = (entry *) malloc(sizeof(entry));
    printf("size of entry : %lu bytes\n", sizeof(entry));
    e = pHead;
    e->pNext = NULL;
#if OPT==2
    //hash variabe
#define TABLE_SIZE 1000
    entry *ht[TABLE_SIZE] = {NULL};
    entry *htHead[TABLE_SIZE] = {NULL};
#endif
#if defined(__GNUC__)
#if OPT==2
    __builtin___clear_cache((char *) htHead, (char *) htHead + TABLE_SIZE*sizeof(entry));
#else
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
#endif
    clock_gettime(CLOCK_REALTIME, &start);

    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
#if OPT==2
        unsigned int index = hashing(line,TABLE_SIZE);
        if(ht[index]==NULL) {
            htHead[index] = (entry *) malloc(sizeof(entry));
            strcpy(htHead[index]->lastName, line);
            ht[index] = htHead[index];
        } else
            ht[index] = append(line,ht[index]);
#else
        e = append(line, e);
#endif
    }
    printf("pHead lastName:%s\n",pHead->lastName);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time1 = diff_in_second(start, end);

    /* close file as soon as possible */
    fclose(fp);

    e = pHead;

    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";
#if OPT==2
    int hash_addr = hashing(input,TABLE_SIZE);
    e = htHead[hash_addr];
#else
    e = pHead;
#endif
    assert(findName(input, e) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(findName(input, e)->lastName, "zyxel"));

#if defined(__GNUC__)
#if OPT==2
    __builtin___clear_cache((char *) htHead, (char *) htHead + TABLE_SIZE*sizeof(entry));
#else
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
#endif
    /* compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    findName(input, e);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time2 = diff_in_second(start, end);

    FILE *output;
#if OPT==1
    output = fopen("opt1.txt", "a");
#elif OPT==2
    output = fopen("opt2.txt", "a");
#else
    output = fopen("orig.txt", "a");
#endif
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);

    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);
#if OPT==2
    for(i=0; i<TABLE_SIZE; i++) {
        if(htHead[i]->pNext)
            free(htHead[i]->pNext);
    }
#else
    if (pHead->pNext) free(pHead->pNext);
    free(pHead);
#endif
    return 0;
}
