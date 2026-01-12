#include <math.h>
#include <stdbool.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int get_cpu_count() {
    long nprocs = -1;
    long nprocs_max = -1;
#ifdef _WIN32
#ifndef _SC_NPROCESSORS_ONLN
    SYSTEM_INFO info;
    GetSystemInfo(&info);
#define sysconf(a) info.dwNumberOfProcessors
#define _SC_NPROCESSORS_ONLN
#endif
#endif
#ifdef _SC_NPROCESSORS_ONLN
    nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1)
    {
        fprintf(stderr, "Could not determine number of CPUs online:\n%s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
    }
    nprocs_max = sysconf(_SC_NPROCESSORS_CONF);
    if (nprocs_max < 1)
    {
        fprintf(stderr, "Could not determine number of CPUs configured:\n%s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
    }
    printf ("%ld of %ld processors online\n",nprocs, nprocs_max);
    return nprocs;
#else
    fprintf(stderr, "Could not determine number of CPUs");
    exit (EXIT_FAILURE);
#endif
}

struct run_data{
    long start;
    long end;
    long jump;
    FILE* OUTPUT_FILE;
};

typedef struct {
    struct run_data* rd;
    long offset;
} Thr_args;

void swapElements(long* x, long* y)
{
    long temp = *x;
    *x = *y;
    *y = temp;
}
// Partition function
long partition (long arr[], long lowIndex, long highIndex)
{
    int pivotElement = arr[highIndex];
    int i = (lowIndex - 1);
    for (int j = lowIndex; j <= highIndex- 1; j++)
    {
        if (arr[j] <= pivotElement)
        {
            i++;
            swapElements(&arr[i], &arr[j]);
        }
    }
    swapElements(&arr[i + 1], &arr[highIndex]);
    return (i + 1);
}
// QuickSort Function
void quickSort(long arr[], long lowIndex, long highIndex)
{
    if (lowIndex < highIndex)
    {
        long pivot = partition(arr, lowIndex, highIndex);
        // Separately sort elements before & after partition
        quickSort(arr, lowIndex, pivot - 1);
        quickSort(arr, pivot + 1, highIndex);
    }
}

void* calculate_primes(void *arg) {
    Thr_args *ta = (Thr_args*)arg;
    for (long i = (ta->rd->start)+(ta->offset); i <= ta->rd->end; i+=ta->rd->jump) {
        long limit = sqrt(i);
        bool isPrime = true;
        for (long j = 2; j <= limit; j++) {
            if (i%j == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            fprintf(ta->rd->OUTPUT_FILE,"%ld\n",i);
        }
    }
    return NULL;
}

int main() {


    int cpu_count = get_cpu_count();
    if (cpu_count < 1) {
        fprintf(stderr, "Could not determine number of CPUs online:");
        exit(-300);
    }
    pthread_t threads[cpu_count];
    struct run_data rd = {2, 1000000000, cpu_count, fopen("output.txt", "w")};
    if (rd.OUTPUT_FILE == NULL) {
        fprintf(stderr,"Could not open output.txt for calculations");
        exit(-404);
    }
    Thr_args targs[cpu_count];

    clock_t start= clock(), end = 0;

    for (int i = 0; i < cpu_count; i++) {
        targs[i].rd = &rd;
        targs[i].offset = i;
        printf("Thread %d starting...\n",i);
        pthread_create(&threads[i], NULL,calculate_primes,&targs[i]);
    }
    for (int i = 0; i < cpu_count; i++) {
        pthread_join(threads[i], NULL);
    }
    end = clock();
    printf("Elapsed calculating time: %Lf ms\n",( long double )(end-start)/CLOCKS_PER_SEC*1000);
    fclose(rd.OUTPUT_FILE);
    rd.OUTPUT_FILE = NULL;

    FILE* data_file = fopen("output.txt", "r");
    if (data_file == NULL) {
        fprintf(stderr,"Could not open output.txt for sorting");
        exit(-404);
    }

    char line[1024];
    long *primes = calloc(1000, sizeof(long));
    if (primes == NULL) {
        fprintf(stderr,"Could not allocate primes storage place");
        exit(-404);
    }
    long loaded_primes = 0, storage_size = 1000;
    while (fgets(line, 1024, data_file) != NULL) {
        line[strcspn(line, "\n")] = 0;
        char* endptr = NULL;

        primes[loaded_primes] = strtol(line,&endptr,10);
        if (endptr == line) {
            fprintf(stderr,"Could not convert line to long");
        }else if ( *endptr != '\0' ) {
            fprintf(stderr,"Invalid character %c\n", *endptr);
        }else {
            loaded_primes++;
            if (loaded_primes >= storage_size-1) {
                long* tmp = realloc(primes,(storage_size+1000)*sizeof(long));
                if (tmp == NULL) {
                    fprintf(stderr,"Could not reallocate primes storage place");
                    exit(-404);
                }

                primes = tmp;
                storage_size += 1000;
            }
        }
    }
    fclose(data_file);
    data_file = NULL;
    printf("Loaded %ld primes\nSorting...\n",loaded_primes);
    start = clock();
    quickSort(primes, 0, loaded_primes-1);
    end = clock();
    printf("Elapsed sorting time: %Lf ms\n",( long double )(end-start)/CLOCKS_PER_SEC*1000);

    data_file = fopen("output.txt", "w");
    for (int i = 0; i < loaded_primes; i++) {
        fprintf(data_file,"%ld\n",primes[i]);
    }


    free(primes);
    primes = NULL;
}
