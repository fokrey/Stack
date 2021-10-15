#include "Stack.h"

int stack_ctor (Stack *stack, const long start_capacity) {
    assert (stack != nullptr);

    dumpfile = fopen ("../dump.txt", "w");
    if (dumpfile == nullptr) {
        perror ("Error in opening dumpfile\n");
        return ERROR_OPEN_FILE;
    }

    stack->size = 0;
    if (start_capacity < 0) {
        perror ("Capacity under zero\n");
        return CAPACITY_LESS_ZERO;
    }

    stack->start_capacity = start_capacity;
    stack->capacity = stack->start_capacity;

    stack->data = (elem_t *) calloc (stack->capacity * sizeof (elem_t) + 2 * sizeof (canary_t), sizeof (char));
    assert (stack->data != nullptr);

    stack->canary_left = (canary_t *) stack->data;   //data[0] is left canary
    *stack->canary_left = CANARY_DATA;

    stack->data = (elem_t *)((char *) stack->data + sizeof (canary_t)); //move pointer to data (shift to size of 1 canary)

    stack->canary_right = (canary_t *) (stack->data + stack->capacity);
    *stack->canary_right = CANARY_DATA;
    stack->hash = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));

    STACK_NOT_OK (stack);
    return NO_ERROR;
}

void stack_dtor (Stack *stack) {
    STACK_NOT_OK(stack);

    memset ((char*)stack->data - sizeof (canary_t), POISON, stack->capacity * sizeof (elem_t) + 2 * sizeof (canary_t));

    free ((char *)(stack->data) - sizeof (canary_t));
    stack->size = 0;
    stack->capacity = 0;
    stack->canary_left = nullptr;
    stack->canary_right = nullptr;
    stack->hash = POISON;

    fclose (dumpfile);
}

int stack_push (Stack *stack, elem_t value) {
    STACK_NOT_OK (stack);

    if (stack->size == stack->capacity) {
        int status = stack_resize (stack, UP);
        if (status != NO_ERROR) {
            return status;
        }
        stack_resize (stack, UP);
    }

    stack->data[stack->size++] = value;
    stack->hash = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));

    STACK_NOT_OK (stack);
    return NO_ERROR;
}

elem_t stack_pop (Stack *stack) {
    STACK_NOT_OK (stack);

    if (stack->size <= cap_decr (stack->capacity) && stack->size > stack->start_capacity) {
        int status = stack_resize (stack, DOWN);
        if (status != NO_ERROR) {
            return status;
        }
    }
    stack->size--;
    stack->hash = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));

    STACK_NOT_OK (stack);
    return stack->data[stack->size];
}

size_t cap_incr (size_t capacity) {
    return 2 * capacity;
}

size_t cap_decr (size_t capacity) {
    return capacity / 2;
}

int stack_resize (Stack *stack, FLAG up_or_down) {
    STACK_NOT_OK (stack);

    size_t new_size = 0;
    if (up_or_down == UP) {
        new_size = cap_incr (stack->capacity);
        stack->capacity *= 2;
    }
    if (up_or_down == DOWN) {
        new_size = cap_decr (stack->capacity);
        stack->capacity /= 2;
    }

    elem_t *temp = stack->data;
    stack->data = (elem_t *) ((char *) stack->data - sizeof (canary_t));  //move pointer to left canary
    stack->data = (elem_t *) realloc (stack->data, new_size * sizeof (elem_t) + 2 * sizeof (canary_t));

    if (stack->data == nullptr) {
        perror ("Stack data is nullptr");
        stack->data = temp;
        return NULL_PTR;
    }

    stack->canary_left = (canary_t *) stack->data;
    *stack->canary_left = CANARY_DATA;

    stack->data = (elem_t  *)((char *) stack->data + sizeof (canary_t)); //move pointer to data

    stack->canary_right = (canary_t *) (stack->data + stack->capacity);
    *stack->canary_right = CANARY_DATA;
    stack->hash = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));

    STACK_NOT_OK (stack);
    return NO_ERROR;
}

bool check_nullptr (const void *ptr) {
    return ptr == nullptr;
}

int check_size_and_capacity (const Stack *stack) {
    if (stack->size < 0) {
        return SIZE_LESS_ZERO;
    }
    return (stack->size > stack->capacity);
}

int check_canary (canary_t *canary) {
    return (*(canary) != CANARY_DATA);
}

int check_hash (const Stack *stack, unsigned hash) {
    return (stack->hash != hash);
}

int stack_not_ok (Stack *stack, const char *file, const char *function, int line) {
    CHECK_NULLPTR (stack);
    CHECK_NULLPTR (stack->data);

    //fprintf (dumpfile, "from file %s function %s was called on %d line\n\n", file, function, line);
    if (check_size_and_capacity (stack)) {
        fprintf (dumpfile, "Stack's capacity is less than size\n");
        STACK_DUMP (stack);
        return CAPACITY_LESS_SIZE;
    }

    if (check_canary (stack->canary_left)) {
        fprintf (dumpfile, "Left canary was changed\n");
        STACK_DUMP (stack);
        return ERROR_LEFT_CANARY;
    }

    if (check_canary (stack->canary_right)) {
        fprintf (dumpfile, "Right canary was changed\n");
        STACK_DUMP (stack);
        return ERROR_RIGHT_CANARY;
    }

    unsigned hash_act = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));
    unsigned hash_exp = stack->hash;

    if (check_hash (stack, hash_act)) {
        fprintf (dumpfile, "Hash mismatch. Expected value: %d, Actual value: %d\n", hash_exp, hash_act);
        //STACK_DUMP (stack);
        return ERROR_HASH;
    }

    return NO_ERROR;
}

unsigned MurmurHash2 (const char * key, unsigned int len) {
    const unsigned m = 0x5bd1e995;
    const unsigned seed = 0;
    const int r = 24;

    uint32_t h = seed ^ len;

    const unsigned char * data = (const unsigned char *)key;
    uint32_t k = 0;

    while (len >= 4) {
        k  = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
        case 3:
            h ^= data[2] << 16;
        case 2:
            h ^= data[1] << 8;
        case 1:
            h ^= data[0];
            h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

int stack_dump (Stack *stack, const char *file, const char *function, int line) {
    CHECK_NULLPTR (stack);
    CHECK_NULLPTR (stack->data);
    fprintf (dumpfile, "DUMP was called from %s file, %s function, %d line\n", file, function, line);
    fprintf (dumpfile, "stack [%p]\n", stack);
    fprintf (dumpfile, "Current size of stack is: %zu\t\t\tcapacity is: %zu\n", stack->size, stack->capacity);
    fprintf (dumpfile, "Hash value is: %d\n", stack->hash);

    fprintf (dumpfile, "Elements of data are:\n");

    print_data_elem (stack);
    fprintf (dumpfile, "left  canary is: %llu\n", *(stack->canary_left));
    fprintf (dumpfile, "right canary is: %llu\n", *(stack->canary_right));
    fprintf (dumpfile, "hash of stack is: %d\n\n\n", stack->hash);
}

void print_data_elem (Stack *stack) {
    for (int i = 0; i < stack->capacity; i++) {
        if (i < stack->size) {
            fprintf(dumpfile, "*[%d] = %d\n", i, stack->data[i]);
        }
        else {
            fprintf (dumpfile, " [%d] = %llu\n", i, stack->data[i]);
        }
    }
}

int stack_test (Stack *stack, const long capacity) {

    int status_ctor = stack_ctor (stack, capacity);
    assert (status_ctor == NO_ERROR);

    for (int i = 0; i < 50; i++) {
        int status_push = stack_push(stack, 2 * i);
        assert (status_push == NO_ERROR);
    }

    for (int i = 0; i < 26; i++) {
        stack_pop (stack);
    }
    STACK_DUMP (stack);

    stack_dtor (stack);
}
