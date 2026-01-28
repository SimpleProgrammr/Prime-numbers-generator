#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdatomic.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

atomic_ullong INDEX = 0;
unsigned long long *ram_storage;

struct main_settings {
    int max_threads;
    unsigned long long start_value;
    unsigned long long end_value;
    int sortingMode;
    char* output_file_name;
    FILE* output_file;
    bool use_ram_storage;
    bool use_gen2_thread_management;
} MAIN_SETTINGS;

int get_cpu_count() {
    int nprocs = -1;
    int nprocs_max = -1;
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
    printf ("%d of %d processors online\n",nprocs, nprocs_max);
    return nprocs;
#else
    fprintf(stderr, "Could not determine number of CPUs");
    exit (EXIT_FAILURE);
#endif
}

struct run_data{
    unsigned long long start;
    unsigned long long end;
    unsigned long long run_for;
};

typedef struct {
    struct run_data* rd;
    unsigned long long offset;
    unsigned long long progress;
} Thr_args;

void swap(unsigned long long* a, unsigned long long* b) {
    unsigned long long temp = *a;
    *a = *b;
    *b = temp;
}

int partition(unsigned long long arr[], unsigned long long low, unsigned long long high) {

    // Initialize pivot to be the first element
    unsigned long long p = arr[low];
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

void quickSort(unsigned long long arr[], unsigned long long low, unsigned long long high) {
    static unsigned long long i = 0;
    if (low < high) {
        i++;
        // call partition function to find Partition Index
        long long pi = partition(arr, low, high);
        pi = pi >=1 ? pi : 1;
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
    printf("Thread #%lld running\n",ta->offset);

    unsigned long long run_for = (ta->rd->start)+(ta->offset)+ta->rd->run_for;
    for (unsigned long long i = (ta->rd->start)+(ta->offset); i <= ta->rd->end && i<=run_for; i+=1) {
        unsigned long long limit = (unsigned long long)(sqrt((double)i)+1);
        bool isPrime = true;
        for (unsigned long long j = 2; j <= limit; j++) {
            if (i%j == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            if (MAIN_SETTINGS.use_ram_storage) {
                unsigned _Atomic long long save_to = atomic_fetch_add(&INDEX, 1);
                ram_storage[save_to] = i;
            }
            else
                fprintf(MAIN_SETTINGS.output_file,"%lld\n",i);
        }
        ta->progress+=1;
    }
    printf("Thread #%lld ended : [Start: %lld, Jump: %lld]\n",ta->offset,ta->rd->start,ta->rd->run_for);
    fflush(stdout);
    return NULL;
}

atomic_ullong NUMBER = 0;

void* calculate_primes_gen2(void *arg) {
    Thr_args *ta = (Thr_args*)arg;
    printf("Thread #%lld running\n",ta->offset);
    unsigned _Atomic long long i = atomic_fetch_add(&NUMBER,1);
    while (i <= ta->rd->end) {
        i = atomic_fetch_add(&NUMBER,1);
        unsigned long long limit = (unsigned long long)(sqrt((double)i)+1);
        bool isPrime = true;
        for (unsigned long long j = 2; j <= limit; j++) {
            if (i%j == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            if (MAIN_SETTINGS.use_ram_storage) {
                unsigned _Atomic long long save_to = atomic_fetch_add(&INDEX, 1);
                ram_storage[save_to] = i;
            }
            else
                fprintf(MAIN_SETTINGS.output_file,"%lld\n",i);
        }
        ta->progress+=1;
    }
    printf("Thread #%lld ended.", ta->offset);
    fflush(stdout);
    return NULL;
}

bool post_run = true;
void * post_progress(void * arg) {
    Thr_args *ta = (Thr_args*)arg;
    unsigned long long fullRange = ta->rd->end;
    const int arrLen = get_cpu_count();

    while (1) {
        if (!post_run)
            return NULL;

        unsigned long long progress = 0;

        for (int i = 0; i < arrLen; i++) {
            progress += ta[i].progress;
        }
        double percent = (double)progress / (double)fullRange * 100.;
        printf("Progress %f%%\r",percent);
        fflush(stdout);
        fflush(MAIN_SETTINGS.output_file);
#ifdef _WIN32
        Sleep(500);
#else
        usleep(500*1000);
#endif
    }
}

void bubble_sort(unsigned long long arr[], unsigned long long lowIndex, unsigned long long highIndex) {
    unsigned long long lp;
    do {
        lp = 0;
        for (unsigned long long i = lowIndex; i < highIndex; i++) {
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

bool setup(int argc, char* argv[]) {
    MAIN_SETTINGS.start_value = 2;
    NUMBER = MAIN_SETTINGS.start_value;
    MAIN_SETTINGS.end_value = 0;
    MAIN_SETTINGS.max_threads = get_cpu_count();
    MAIN_SETTINGS.output_file_name = NULL;
    MAIN_SETTINGS.sortingMode = 2;
    MAIN_SETTINGS.use_ram_storage = false;
    MAIN_SETTINGS.use_gen2_thread_management = true;

    char* endptr = NULL;
    for (int i = 1; i < argc; i++) {
        if (strlen(argv[i]) > 1) {
            if (argv[i][0] == '-') {
                switch (argv[i][1]) {
                    case 'h':
                        //TODO:help menu
                        break;
                    case 'l': //legacy thread management
                        MAIN_SETTINGS.use_gen2_thread_management = false;
                        break;
                    case 'r':
                        MAIN_SETTINGS.use_ram_storage = true;
                        printf("Warning!!!: Now you are using RAM storage for primes\n");
                    case 'q':
                        MAIN_SETTINGS.sortingMode = 1;
                    break;
                    case 'b':
                        MAIN_SETTINGS.sortingMode = 2;
                        break;
                    case 'e':
                        i++;
                        MAIN_SETTINGS.end_value = strtoll(argv[i], &endptr, 10);
                        break;
                    case 's':
                        i++;
                        MAIN_SETTINGS.start_value = (int)strtol(argv[i], &endptr, 10);
                        NUMBER = MAIN_SETTINGS.start_value - 1;
                        if (MAIN_SETTINGS.start_value < 2) {
                            fprintf(stderr,"Too small value for start_value");
                            return false;
                        }
                        break;
                    case 't':
                        i++;
                        MAIN_SETTINGS.max_threads = (int)strtol(argv[i], &endptr, 10);
                        if (MAIN_SETTINGS.max_threads <= 0) {
                            fprintf(stderr,"Invalid value for max_threads");
                            return false;
                        }
                        break;
                    case 'o':
                        i++;
                        MAIN_SETTINGS.output_file_name = argv[i];
                        MAIN_SETTINGS.output_file = fopen( MAIN_SETTINGS.output_file_name,"w");
                        if (MAIN_SETTINGS.output_file == NULL) {
                            fprintf(stderr,"Could not open output file");
                            return false;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    if (MAIN_SETTINGS.end_value < MAIN_SETTINGS.start_value) {
        fprintf(stderr,"Error: bad work values\n");
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (!setup(argc, argv)) {
        printf("Setup failed\n");
        return -1;
    }

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    pthread_t threads[MAIN_SETTINGS.max_threads];
    //Setting running data
    unsigned long long thread_part = MAIN_SETTINGS.end_value/MAIN_SETTINGS.max_threads;
    struct run_data rd = {MAIN_SETTINGS.start_value, MAIN_SETTINGS.end_value, thread_part};
    Thr_args *targs = calloc(MAIN_SETTINGS.max_threads, sizeof(Thr_args));
    unsigned long long predicted_primes_amount = (MAIN_SETTINGS.end_value - MAIN_SETTINGS.start_value);
    if (MAIN_SETTINGS.use_ram_storage) {
        ram_storage = calloc(predicted_primes_amount, sizeof(unsigned long long));
        if (ram_storage == NULL) {
            fprintf(stderr,"Could not allocate memory for ram_storage");
            return -1;
        }
    }

    //Starting timer
    clock_t start = clock(), end = 0;
    switch (MAIN_SETTINGS.use_gen2_thread_management) {
        case false:
            for (int i = 0; i < MAIN_SETTINGS.max_threads; i++) {
                //Combining arguments
                targs[i].rd = &rd;
                targs[i].offset = thread_part*i;
                targs[i].progress = 0;
                pthread_create(&threads[i], NULL,calculate_primes,&targs[i]);
            }
            break;
        case true:
            for (int i = 0; i < MAIN_SETTINGS.max_threads; i++) {
                //Combining arguments
                targs[i].rd = &rd;
                targs[i].offset = i;
                targs[i].progress = 0;
                pthread_create(&threads[i], NULL,calculate_primes_gen2,&targs[i]);
            }
            break;
    }
    fflush(stdout);
    //Launching posting
    pthread_t post_progress_thread;
    pthread_create(&post_progress_thread, NULL, post_progress, targs);
    fflush(stdout);
    for (int i = 0; i < MAIN_SETTINGS.max_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_cancel(post_progress_thread);
    post_run = false;

    end = clock();
    printf("Elapsed calculating time: %Lf ms\n",( long double )(end-start)/CLOCKS_PER_SEC*1000);
    fclose(MAIN_SETTINGS.output_file);
    MAIN_SETTINGS.output_file = NULL;
    free(targs);
    targs = NULL;


    unsigned long long *primes = calloc(1000, sizeof(unsigned long long));
        if (primes == NULL) {
            fprintf(stderr,"Could not allocate primes storage place");
            exit(-404);
        }
    unsigned _Atomic long long loaded_primes;
    if (MAIN_SETTINGS.use_ram_storage)
        loaded_primes = atomic_fetch_add(&INDEX, 0);
    else
        loaded_primes = 0;
    FILE* data_file = NULL;


    if (!MAIN_SETTINGS.use_ram_storage) {
        data_file = fopen(MAIN_SETTINGS.output_file_name, "r");
        if (data_file == NULL) {
            fprintf(stderr,"Could not open output.txt for sorting");
            exit(-404);
        }

        char line[1024];

        unsigned long long storage_size = 1000;
        while (fgets(line, 1024, data_file) != NULL) {
            line[strcspn(line, "\n")] = 0;
            char* endptr = NULL;

            primes[loaded_primes] = strtol(line,&endptr,10);
            if (endptr == line) {
                fprintf(stderr,"Could not convert line to unsigned long long");
            }else if ( *endptr != '\0' ) {
                fprintf(stderr,"Invalid character %c\n", *endptr);
            }else {
                loaded_primes++;
                if (loaded_primes >= storage_size-1) {
                    unsigned long long* tmp = realloc(primes,(storage_size+1000)*sizeof(unsigned long long));
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
    }
    if (MAIN_SETTINGS.use_ram_storage)
        primes = ram_storage;

    start = clock();
    if (MAIN_SETTINGS.sortingMode == 1)
        quickSort(primes, 0, loaded_primes-1);
    else
        bubble_sort(primes, 0, loaded_primes-1);
    end = clock();
    printf("Elapsed sorting time: %Lf ms\n",( long double )(end-start)/CLOCKS_PER_SEC*1000);

    printf("Saving to file...\n");
    data_file = fopen(MAIN_SETTINGS.output_file_name, "w");
    for (unsigned long long i = 0; i < loaded_primes; i++) {
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
    ram_storage = NULL;
}