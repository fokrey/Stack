#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

typedef int               elem_t;
typedef long long int     canary_t;
const unsigned long int POISON   = 0xDED32BAD;
const canary_t CANARY_DATA       = 0xBADBABE;
const canary_t CANARY_STACK      = 0xBADBEDA;
static FILE *dumpfile            = nullptr;

struct Stack {
    canary_t canary_st_left = 0;
    size_t size = 0;
    long start_capacity = 0;
    size_t capacity = 0;
    elem_t *data = nullptr;
    unsigned hash_data = 0;
    unsigned hash_stack = 0;
    canary_t canary_data_left  = 0; //canary for data
    canary_t canary_data_right = 0; //canary for data
    canary_t canary_st_right = 0;
};
const int stack_size = sizeof (Stack);

enum FLAG {
    DOWN = 0,
    UP = 1
};

enum ERROR {
    NO_ERROR                 = 0,
    NULL_PTR                 = 1,
    SIZE_LESS_ZERO           = 3,
    CAPACITY_LESS_ZERO       = 4,
    CAPACITY_LESS_SIZE       = 5,
    ERROR_OPEN_FILE          = 6,
    ERROR_LEFT_DATA_CANARY   = 7,
    ERROR_RIGHT_DATA_CANARY  = 8,
    ERROR_LEFT_STACK_CANARY  = 9,
    ERROR_RIGHT_STACK_CANARY = 10,
    ERROR_HASH               = 11
};

#define OPEN_DUMPFILE() open_dumpfile ();

#define CHECK_NULLPTR(ptr) \
    if (check_nullptr (ptr)) \
        return NULL_PTR

#define STACK_DUMP(stackptr) stack_dump(stackptr, __FILE__, __FUNCTION__, __LINE__)

#define STACK_NOT_OK(stackptr)                                                          \
    {                                                                                   \
        int error_number = stack_not_ok(stackptr, __FILE__, __FUNCTION__, __LINE__);    \
        if (error_number!= NO_ERROR)                                                    \
            return error_number;                                                        \
    }



int stack_ctor (Stack *stack, const long start_capacity);
int stack_dtor (Stack *stack);
int stack_push (Stack *stack, elem_t value) ;
elem_t stack_pop (Stack *stack);
size_t cap_incr (size_t capacity);
size_t cap_decr (size_t capacity);
int stack_resize (Stack *stack, FLAG up_or_down);
unsigned calc_stack_hash (Stack *stack);
bool check_nullptr (const void *ptr);
int check_size_and_capacity (const Stack *stack);
int open_dumpfile ();
int stack_not_ok (Stack *stack, const char *file, const char *function, int line);
unsigned MurmurHash2 (const char * key, unsigned int len);
int stack_dump (Stack *stack, const char *file, const char *function, int line);
void print_data_elem (Stack *stack);
int stack_test (Stack *stack, const long);