#include "cachelab.h"
#include <stdint.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>    // errno
#include <stdlib.h>   // atol exit
#include <sys/time.h>

typedef struct
{
    uint64_t s; // number of sets index's bits
    uint64_t b; // number of blocks index's bits
    uint64_t S; // number of sets
    uint64_t E; // number of lines
    FILE *tracefile; // file pointer
}Options;

Options  GetOptions(int argc, char* const argv[]);



/*
Data structure:

+--------------+
| (cache0)     |        +--------------+
|  sets        +--------> (set0)       |        +-----------+
+--------------+        |  lines       +--------> (line0)   |
                        +--------------+        |  valid    |
                        | (set1)       |        |  tag      |
                        |  lines       |        |  counter  |
                        +--------------+        +-----------+
                        | (set2)       |        | (line1)   |
                        |  lines       |        |  valid    |
                        +--------------+        |  tag      |
                        | (setX)       |        |  counter  |
                        |  lines       |        +-----------+
                        +--------------+        | (lineX)   |
                                                |  valid    |
                                                |  tag      |
                                                |  counter  |
                                                +-----------+
*/

typedef uint64_t Bool;
#define  false 0;
#define  true 1;

typedef struct {
    Bool valid; // weather this is valid or not
    uint64_t tag; // identifier to choose line
    uint64_t  timeStamp;  // for LRU strategy
} Line;

typedef struct {
    Line *lines;
    uint64_t length;
} Set;

typedef struct {
    Set *sets;
    uint64_t  length;
} Cache;

typedef struct
{
    int hit;
    int miss;
    int eviction;
}Result;

Cache CreateCache(Options opt);
Result UpdateCache(Cache cache, Options opt);
void UpdateSet(Set set, Result *result, uint64_t tag);
uint64_t GetSystemTime();
void DestroyCache(Cache cache);


// argv is a constant pointer ,type is char**
int main(int argc,    char*  const argv[]){
//    **argv = 'b';  // cant update the value of the address that argv point to
    Options  opt = GetOptions(argc,argv);
    Cache cache = CreateCache(opt);
    Result res = UpdateCache(cache,opt);
    DestroyCache(cache);
    printSummary(res.hit, res.miss, res.eviction);
    return 0;
}

Options  GetOptions(int argc, char* const argv[]){
    const char *help_message = "Usage: \"Your complied program\" [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n" \
                               "<s> <E> <b> should all above zero and below 64.\n" \
                               "Complied with std=c99\n";
    const char *command_options = "hvs:E:b:t:";
    Options opt;
    char ch;
    while((ch = getopt(argc, argv, command_options)) != -1){
        switch(ch){
            case 'h':{
                printf("%s", help_message);
                exit(EXIT_SUCCESS);
            }
            case 's':{
                if (atol(optarg) <= 0){  // at least two sets
                    printf("%s", help_message);
                    exit(EXIT_FAILURE);
                }
                opt.s = atol(optarg);
                opt.S = 1 << opt.s;
                break;
            }

            case 'E':{
                if (atol(optarg) <= 0){
                    printf("%s", help_message);
                    exit(EXIT_FAILURE);
                }
                opt.E = atol(optarg);
                break;
            }

            case 'b':{
                if (atol(optarg) <= 0){  // at least two sets
                    printf("%s", help_message);
                    exit(EXIT_FAILURE);
                }
                opt.b = atol(optarg);
                break;
            }

            case 't':{
                if ((opt.tracefile = fopen(optarg, "r")) == NULL){
                    perror("Failed to open tracefile");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            default:{
                printf("%s", help_message);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (opt.s == 0 || opt.b ==0 || opt.E == 0 || opt.tracefile == NULL){
        printf("%s", help_message);
        exit(EXIT_FAILURE);
    }

    return opt;
}

Cache CreateCache(Options opt) {
    Cache cache;
    if ((cache.sets = calloc(opt.S, sizeof(Set))) == NULL) { // initialize sets
        perror("Failed to create sets of cache");
        exit(EXIT_FAILURE);
    }
    cache.length = opt.S;

    for (int i = 0; i < opt.S; ++i) {
        if ((cache.sets[i].lines = calloc(opt.E,sizeof(Line))) == NULL) {  // initialize the lines in set
            char errStr[40];
            sprintf(errStr,"Failed to create lines of set %d \n",i);
            perror(errStr);
            exit(EXIT_FAILURE);
        }
        cache.sets[i].length = opt.E;
    }

    return cache;
}

Result UpdateCache(Cache cache, Options opt) {
    Result result = {0,0,0};

    char instruction;
    uint64_t  address;
    uint64_t  set_index_mask = (1<<opt.s) -1;
    while ((fscanf(opt.tracefile," %c %lx%*[^\n]", &instruction,&address)) == 2) {

        if (instruction == 'I'){
            continue;
        }

        uint64_t  set_index = (address >> opt.b) & set_index_mask;
        uint64_t tag = (address >>opt.b) >> opt.s;
        Set set = cache.sets[set_index];
        if (instruction == 'L' || instruction == 'S'){
            UpdateSet(set, &result, tag);
        }


        if (instruction == 'M'){
            // // modify is treated as a load followed by a store to the same address.
            UpdateSet(set, &result, tag);
            UpdateSet(set, &result, tag);
        }
    }


    return result;
}

void DestroyCache(Cache cache) {

    for (int i = 0; i < cache.length; ++i) {
        free(cache.sets[i].lines);
    }
    free(cache.sets);
}

uint64_t GetSystemTime(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000.0 * tv.tv_sec + tv.tv_usec * 1000000;
}

void UpdateSet(Set set, Result *result, uint64_t tag) {

    Bool hitFlag = false;
    for(int i = 0 ; i < set.length ; i++ ){
        if (set.lines[i].tag == tag && set.lines[i].valid) { // cache hit !
            hitFlag = true;
            result->hit ++;
            set.lines[i].timeStamp = GetSystemTime();
            break;
        }
    }

    if (!hitFlag) {
        // cache miss case
        result->miss ++;
        Bool find_invalid_line = false;
        for (int i = 0; i < set.length; ++i) {
            if ( !set.lines[i].valid ) { // find an invalid line to place new block data
                set.lines[i].valid = true;
                set.lines[i].timeStamp = GetSystemTime();
                set.lines[i].tag = tag;
                find_invalid_line = true;
                break;
            }
        }

        if (!find_invalid_line) {
            // no invalid cache line find, cache is full, use lru policy to evict old cache block
            result->eviction ++;

            uint64_t  oldest_timestamp = UINT64_MAX;
            int line_idx_to_replace = 0 ;
            for (int i = 0; i < set.length; ++i) { // find the oldest line
                if (set.lines[i].timeStamp < oldest_timestamp) {
                    oldest_timestamp = set.lines[i].timeStamp ;
                    line_idx_to_replace = i;
                }
            }
            // set new data to the oldest line
            set.lines[line_idx_to_replace].timeStamp = GetSystemTime();
            // set.lines[line_idx_to_replace].valid = true; //  cache is full , so the oldest line is alway valid
            set.lines[line_idx_to_replace].tag = tag;
        }
    }
}




