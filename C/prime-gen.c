#include <errno.h>
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

long long get_cpu_count() {
    long long nprocs = -1;
    long long nprocs_max = -1;
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
    printf ("%lld of %lld processors online\n",nprocs, nprocs_max);
    return nprocs;
#else
    fprintf(stderr, "Could not determine number of CPUs");
    exit (EXIT_FAILURE);
#endif
}

struct run_data{
    long long start;
    long long end;
    long long jump;
    FILE* OUTPUT_FILE;
};

typedef struct {
    struct run_data* rd;
    long long offset;
    long long progress;
} Thr_args;

void swap(long long* a, long long* b) {
    long long temp = *a;
    *a = *b;
    *b = temp;
}

int partition(long long arr[], long long low, long long high) {

    // Initialize pivot to be the first element
    long long p = arr[low];
    long long i = low;
    long long j = high;

    while (i < j) {

        // Find the first element greater than
        // the pivot (from starting)
        while (arr[i] <= p && i <= high - 1) {
            i++;
        }

        // Find the first element smaller than
        // the pivot (from last)
        while (arr[j] > p && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[low], &arr[j]);
    return j;
}

void quickSort(long long arr[], long long low, long long high) {
    static long long i = 0;
    if (low < high) {
        i++;
        // call partition function to find Partition Index
        long long pi = partition(arr, low, high);
        if (i%100 == 0) {
            printf("Quick sorting...%lld\r",i);
            fflush(stdout);
        }
        // Recursively call quickSort() for left and right
        // half based on Partition Index
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
        i--;
    }
}

void* calculate_primes(void *arg) {
    Thr_args *ta = (Thr_args*)arg;
    for (long long i = (ta->rd->start)+(ta->offset); i <= ta->rd->end; i+=ta->rd->jump) {
        long long limit = (long long)(sqrt((double)i));
        bool isPrime = true;
        for (long long j = 2; j <= limit; j++) {
            if (i%j == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            fprintf(ta->rd->OUTPUT_FILE,"%lld\n",i);
        }
        ta->progress+=1;
    }
    return NULL;
}

bool post_run = true;
void * post_progress(void * arg) {
    Thr_args *ta = (Thr_args*)arg;
    long long fullRange = ta->rd->end;
    const int arrLen = get_cpu_count();

    while (1) {
        if (!post_run)
            return NULL;

        long long progress = 0;

        for (int i = 0; i < arrLen; i++) {
            progress += ta[i].progress;
        }
        double percent = (double)progress / (double)fullRange * 100.;
        printf("Progress %f%%\r",percent);
        fflush(stdout);
#ifdef _WIN32
        Sleep(500);
#else
        usleep(500*1000);
#endif
    }
}

void bubble_sort(long long arr[], long long lowIndex, long long highIndex) {
    long long lp;
    do {
        lp = 0;
        for (long long i = lowIndex; i < highIndex; i++) {
            if (arr[i] > arr[i+1]) {
                swap(&arr[i],&arr[i+1]);
                lp++;
            }
            if (lp%100 == 0) {
                printf("Bubble sorting...%lld\r",lp);
                fflush(stdout);
            }
        }
    }while (lp>0);
}

int main(int argc, char* argv[]) {

    long long  limit = -1;
    if (argc > 2)
        limit = strtoll(argv[2],NULL,10);
    if (limit == -1) {
        printf("Usage: ./file [(q/b)sorting option] [calculation limit]\n");
    }
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    long long cpu_count = get_cpu_count();
    pthread_t threads[cpu_count];
    //Setting running data
    struct run_data rd = {2, limit, cpu_count, fopen("output.txt", "w")};
    if (rd.OUTPUT_FILE == NULL) {
        fprintf(stderr,"Could not open output.txt for calculations");
        exit(-404);
    }
    Thr_args *targs = calloc(cpu_count, sizeof(Thr_args));

    //Starting timer
    clock_t start = clock(), end = 0;
    printf("Threads ");
    //Starting threads
    for (int i = 0; i < cpu_count; i++) {
        //Combining arguments
        targs[i].rd = &rd;
        targs[i].offset = i;
        targs[i].progress = 0;
        printf("%d, ",i);
        pthread_create(&threads[i], NULL,calculate_primes,&targs[i]);
    }
    //Launching posting
    pthread_t post_progress_thread;
    pthread_create(&post_progress_thread, NULL, post_progress, targs);
    printf("running...\n");
    fflush(stdout);
    for (int i = 0; i < cpu_count; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_cancel(post_progress_thread);
    post_run = false;

    end = clock();
    printf("Elapsed calculating time: %Lf ms\n",( long double )(end-start)/CLOCKS_PER_SEC*1000);
    fclose(rd.OUTPUT_FILE);
    rd.OUTPUT_FILE = NULL;
    free(targs);
    targs = NULL;

    FILE* data_file = fopen("output.txt", "r");
    if (data_file == NULL) {
        fprintf(stderr,"Could not open output.txt for sorting");
        exit(-404);
    }

    char line[1024];
    long long *primes = calloc(1000, sizeof(long long));
    if (primes == NULL) {
        fprintf(stderr,"Could not allocate primes storage place");
        exit(-404);
    }
    long long loaded_primes = 0, storage_size = 1000;
    while (fgets(line, 1024, data_file) != NULL) {
        line[strcspn(line, "\n")] = 0;
        char* endptr = NULL;

        primes[loaded_primes] = strtol(line,&endptr,10);
        if (endptr == line) {
            fprintf(stderr,"Could not convert line to long long");
        }else if ( *endptr != '\0' ) {
            fprintf(stderr,"Invalid character %c\n", *endptr);
        }else {
            loaded_primes++;
            if (loaded_primes >= storage_size-1) {
                long long* tmp = realloc(primes,(storage_size+1000)*sizeof(long long));
                if (tmp == NULL) {
                    fprintf(stderr,"Could not reallocate primes storage place");
                    free(primes);
                    exit(-404);
                }

                primes = tmp;
                storage_size += 1000;
            }
        }
    }
    fclose(data_file);
    data_file = NULL;
    printf("Loaded %lld primes\nSorting...\n",loaded_primes);
    start = clock();
    if (argc > 1 && strcmp(argv[1],"q") == 0)
        quickSort(primes, 0, loaded_primes-1);
    else
        bubble_sort(primes, 0, storage_size-1);
    end = clock();
    printf("Elapsed sorting time: %Lf ms\n",( long double )(end-start)/CLOCKS_PER_SEC*1000);

    printf("Saving to file...\n");
    data_file = fopen("output.txt", "w");
    for (long long i = 0; i < loaded_primes; i++) {
        fprintf(data_file,"%lld\n",primes[i]);
        if (i%100 == 0) {
            printf("Saving...%lld\r",i);
            fflush(stdout);
        }
    }
    fclose(data_file);
    printf("Saved!\n");

    free(primes);
    primes = NULL;
}
