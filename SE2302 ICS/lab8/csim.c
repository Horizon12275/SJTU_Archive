//赵楷越 522031910803
// This program is a cache simulator that simulates the behavior of a cache memory. 
// It reads a valgrind memory trace file as input and simulates the hit/miss behavior of 
// a cache memory on this trace. It can be used to evaluate the performance of different
// cache organizations and replacement strategies.
#include "cachelab.h"
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>

// Variables Definition And Initialization
int h = 0, v = 0, s = 0, E = 0, b = 0, S = 0; 

int Hits = 0, Misses = 0, Evictions = 0;

char input_string[1000];

// Cache Structure
typedef struct
{
    int valid_bit;
    int tag_bit;
    int time_stamp;
} cache_line, *cache_line_pointer, **cache;

cache my_cache = NULL;

// init_cache()
// This function initializes the cache according to the given parameters S and E.
void init_cache()
{
    my_cache = (cache)malloc(sizeof(cache_line_pointer) * S);
    for (int i = 0; i < S; i++)
    {
        my_cache[i] = (cache_line_pointer)malloc(sizeof(cache_line) * E);
        for (int j = 0; j < E; j++)
        {
            my_cache[i][j].valid_bit = 0;
            my_cache[i][j].tag_bit = -1;
            my_cache[i][j].time_stamp = -1;
        }
    }
}

// update(unsigned int address)
// This function updates the cache according to the given address.
void update(unsigned int address)
{
    int setindex_address = (address >> b) & ((-1U) >> (64 - s));
    int tag_address = address >> (b + s);

    int max_time_stamp = INT_MIN;
    int max_time_stamp_index = -1;

    // Find if the address in the cache can be hit
    for (int i = 0; i < E; i++)
    {
        if (my_cache[setindex_address][i].tag_bit == tag_address)
        {
            my_cache[setindex_address][i].time_stamp = 0;
            Hits++;
            return;
        }
    }

    // If not hit, find if there is an empty space
    for (int i = 0; i < E; ++i)
    {
        if (my_cache[setindex_address][i].valid_bit == 0)
        {
            my_cache[setindex_address][i].valid_bit = 1;
            my_cache[setindex_address][i].tag_bit = tag_address;
            my_cache[setindex_address][i].time_stamp = 0;
            Misses++;
            return;
        }
    }

    // If there is no empty space, find the least recently used space
    Evictions++;
    Misses++;

    for (int i = 0; i < E; i++)
    {
        if (my_cache[setindex_address][i].time_stamp > max_time_stamp)
        {
            max_time_stamp = my_cache[setindex_address][i].time_stamp;
            max_time_stamp_index = i;
        }
    }
    my_cache[setindex_address][max_time_stamp_index].tag_bit = tag_address;
    my_cache[setindex_address][max_time_stamp_index].time_stamp = 0;
}

// update_time_stamp()
// This function updates the time stamp of the cache.
void update_time_stamp()
{
    for (int i = 0; i < S; i++)
        for (int j = 0; j < E; j++)
            if (my_cache[i][j].valid_bit == 1)
                my_cache[i][j].time_stamp++;
}

// parse_trace()
// This function parses the trace file and updates the cache.
void parse_trace()
{
    FILE *trace_file = fopen(input_string, "r");
    if (trace_file == NULL)
    {
        printf("file open error");
        exit(-1);
    }
    char operation;      
    unsigned int address;
    int size;            
    while (fscanf(trace_file, " %c %xu,%d\n", &operation, &address, &size) > 0)
    {
        switch (operation)
        {
        case 'M':
            update(address);
            update(address);
            break;
        case 'L':
            update(address);
            break;
        case 'S':
            update(address);
            break;
        default:
            break;
        }
        update_time_stamp();
    }
    fclose(trace_file);
}

// free_cache()
// This function frees the cache.
void free_cache()
{
    for (int i = 0; i < S; i++)
        free(my_cache[i]);
    free(my_cache);
}

// main()
// This is the main function of the program.
int main(int argc, char *argv[])
{
    // Parse the command line arguments
    int opt = -1;

    while ((opt = (getopt(argc, argv, "hvs:E:b:t:"))) != -1)
    {
        switch (opt)
        {
        case 'h':
            break;
        case 'v':
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            strcpy(input_string, optarg);
            break;
        default:
            break;
        }
    }

    // Invalid check
    if (s <= 0 || E <= 0 || b <= 0 || input_string == NULL)
        return -1;

    S = pow(2, s);

    FILE *trace_file = fopen(input_string, "r");
    if (trace_file == NULL)
    {
        printf("file open error");
        exit(-1);
    }

    // Run the cache simulator
    init_cache(); 
    parse_trace();
    free_cache();

    printSummary(Hits, Misses, Evictions);

    return 0;
}