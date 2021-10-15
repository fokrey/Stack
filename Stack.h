#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

typedef int               elem_t;
typedef long long int     canary_t;
const unsigned long int POISON   = 0xDED32BAD;
const canary_t CANARY_DATA       = 0xBADBABE;
static FILE *dumpfile            = nullptr;

struct Stack {
    size_t size = 0;
    long start_capacity = 0;
    size_t capacity = 0;
    elem_t *data = nullptr;
    uint32_t hash = 0;
    canary_t *canary_left = nullptr; //canary for data
    canary_t *canary_right = nullptr; //canary for data
};


#define CHECK_NULLPTR(ptr) \
        if (check_nullptr (ptr)) \
            return NULL_PTR

#define STACK_DUMP(stackptr) stack_dump(stackptr, __FILE__, __FUNCTION__, __LINE__)

#define STACK_NOT_OK(stackptr) stack_not_ok(stackptr, __FILE__, __FUNCTION__, __LINE__)

enum FLAG {
    DOWN = 0,
    UP = 1
};

enum ERROR {
    NO_ERROR           = 0,
    NULL_PTR           = 1,
    SIZE_LESS_ZERO     = 3,
    CAPACITY_LESS_ZERO = 4,
    CAPACITY_LESS_SIZE = 5,
    ERROR_OPEN_FILE    = 6,
    ERROR_LEFT_CANARY  = 7,
    ERROR_RIGHT_CANARY = 8,
    ERROR_HASH         = 9
};

int stack_ctor (Stack *stack, const long start_capacity);
void stack_dtor (Stack *stack);
int stack_push (Stack *stack, elem_t value) ;
elem_t stack_pop (Stack *stack);
size_t cap_incr (size_t capacity);
size_t cap_decr (size_t capacity);
int stack_resize (Stack *stack, FLAG up_or_down);
bool check_nullptr (const void *ptr);
int check_size_and_capacity (const Stack *stack);
int check_canary (canary_t *canary);
int check_hash (const Stack *stack, unsigned hash);
int stack_not_ok (Stack *stack, const char *file, const char *function, int line);
unsigned MurmurHash2 (const char * key, unsigned int len);
int stack_dump (Stack *stack, const char *file, const char *function, int line);
void print_data_elem (Stack *stack);
int stack_test (Stack *stack, const long);