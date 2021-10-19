#include "Stack.h"

int stack_ctor (Stack *stack, const long start_capacity) {
    assert (stack != nullptr);

    stack->size = 0;
    if (start_capacity <= 0) {
        perror ("Capacity under or equals zero\n");
        return CAPACITY_LESS_ZERO;
    }
    stack->canary_st_left = CANARY_STACK;
    stack->start_capacity = start_capacity;
    stack->capacity = stack->start_capacity;

    stack->data = (elem_t *) calloc (stack->capacity * sizeof (elem_t) + 2 * sizeof (canary_t), sizeof (char));
    assert (stack->data != nullptr);

    stack->canary_data_left = *((canary_t *) stack->data);   //data[0] is left canary
    stack->canary_data_left = CANARY_DATA;

    stack->data = (elem_t *)((char *) stack->data + sizeof (canary_t)); //move pointer to data (shift to size of 1 canary)

    stack->canary_data_right = *((canary_t *) (stack->data + stack->capacity));
    stack->canary_data_right = CANARY_DATA;
    stack->canary_st_right = CANARY_STACK;
    stack->hash_data = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));
    stack->hash_stack = 0;
    stack->hash_stack = MurmurHash2 ((const char *) stack, sizeof (*stack));

    STACK_NOT_OK (stack)

    return NO_ERROR;
}

int stack_dtor (Stack *stack) {
    STACK_NOT_OK(stack);

    memset ((char*)stack->data - sizeof (canary_t), POISON, stack->capacity * sizeof (elem_t) + 2 * sizeof (canary_t));

    free ((char *)(stack->data) - sizeof (canary_t));
    stack->size = 0;
    stack->capacity = 0;

    stack->canary_data_left  = 0;
    stack->canary_data_right = 0;

    stack->canary_st_left    = 0;
    stack->canary_st_right   = 0;

    stack->hash_data = POISON;
    stack->hash_stack = POISON;
    return NO_ERROR;
}

int stack_push (Stack *stack, elem_t value) {
    STACK_NOT_OK (stack)

    if (stack->size == stack->capacity) {
        int status = stack_resize (stack, UP);
        if (status != NO_ERROR) {
            return status;
        }
        stack_resize (stack, UP);
    }

    stack->data[stack->size++] = value;
    stack->hash_data = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));
    stack->hash_stack = 0;
    stack->hash_stack = MurmurHash2 ((const char *) stack, stack_size);

    STACK_NOT_OK (stack);
    return NO_ERROR;
}

elem_t stack_pop (Stack *stack) {
    STACK_NOT_OK (stack);

    if (stack->size <= cap_decrease (stack->capacity) && stack->size > stack->start_capacity) {
        int status = stack_resize (stack, DOWN);
        if (status != NO_ERROR) {
            return status;
        }
    }
    stack->size--;
    elem_t pop_elem = stack->data[stack->size];
    stack->data[stack->size] = POISON;

    stack->hash_data = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));
    stack->hash_stack = calc_stack_hash (stack);

    STACK_NOT_OK (stack);
    return pop_elem;
}

size_t cap_increase (size_t capacity) {
    return 2 * capacity;
}

size_t cap_decrease (size_t capacity) {
    return capacity / 4;
}

int stack_resize (Stack *stack, FLAG up_or_down) {
    STACK_NOT_OK (stack);

    size_t new_size = 0;
    if (up_or_down == UP) {
        new_size = cap_increase(stack->capacity);
        stack->capacity = cap_increase (stack->capacity);
    }
    if (up_or_down == DOWN) {
        new_size = cap_decrease (stack->capacity);
        stack->capacity = cap_decrease (stack->capacity);
    }

    elem_t *temp = stack->data;
    stack->data = (elem_t *) ((char *) stack->data - sizeof (canary_t));  //move pointer to left canary
    stack->data = (elem_t *) realloc (stack->data, new_size * sizeof (elem_t) + 2 * sizeof (canary_t));

    if (stack->data == nullptr) {
        perror ("Stack data is nullptr");
        stack->data = temp;
        return NULL_PTR;
    }

    stack->canary_data_left = *((canary_t *) stack->data);
    stack->canary_data_left = CANARY_DATA;

    stack->data = (elem_t  *)((char *) stack->data + sizeof (canary_t)); //move pointer to data

    stack->canary_data_right = *((canary_t *) (stack->data + stack->capacity));
    stack->canary_data_right = CANARY_DATA;
    stack->canary_st_left  = CANARY_STACK;
    stack->canary_st_right = CANARY_STACK;
    stack->hash_data = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));
    stack->hash_stack = calc_stack_hash (stack);

    if (up_or_down == UP) {
        for (int i = stack->size; i < stack->capacity; i++) {
            stack->data[i] = POISON;
        }
    }

    STACK_NOT_OK (stack);
    return NO_ERROR;
}

unsigned calc_stack_hash (Stack *stack) {
    stack->hash_stack = 0;
    return MurmurHash2 ((const char *) stack, stack_size);
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

int stack_not_ok (Stack *stack, const char *file, const char *function, int line) {
    CHECK_NULLPTR (stack);
    CHECK_NULLPTR (stack->data);

    if (check_size_and_capacity (stack)) {
        OPEN_DUMPFILE()
        fprintf (dumpfile, "From file %s function %s was called on %d line\n\n", file, function, line);
        fprintf (dumpfile, "Stack's capacity is less than size\n");
        STACK_DUMP (stack);
        return CAPACITY_LESS_SIZE;
    }

    if (stack->canary_data_left != CANARY_DATA) {
        OPEN_DUMPFILE()
        fprintf (dumpfile, "From file %s function %s was called on %d line\n\n", file, function, line);
        fprintf (dumpfile, "Left canary for data was changed\n");
        STACK_DUMP (stack);
        return ERROR_LEFT_DATA_CANARY;
    }

    if (stack->canary_data_right != CANARY_DATA) {
        OPEN_DUMPFILE()
        fprintf (dumpfile, "From file %s function %s was called on %d line\n\n", file, function, line);
        fprintf (dumpfile, "Right canary for data was changed\n");
        STACK_DUMP (stack);
        return ERROR_RIGHT_DATA_CANARY;
    }

    if ((stack->canary_st_left != CANARY_STACK)) {
        OPEN_DUMPFILE()
        fprintf (dumpfile, "From file %s function %s was called on %d line\n\n", file, function, line);
        fprintf (dumpfile, "Left canary for stack was changed\n");
        STACK_DUMP (stack);
        return ERROR_LEFT_STACK_CANARY;
    }

    if (stack->canary_st_right != CANARY_STACK) {
        OPEN_DUMPFILE()
        fprintf (dumpfile, "From file %s function %s was called on %d line\n\n", file, function, line);
        fprintf (dumpfile, "Right canary for stack was changed\n");
        STACK_DUMP (stack);
        return ERROR_RIGHT_STACK_CANARY;
    }

    unsigned hash_data = MurmurHash2 ((const char *) stack->data, stack->size * sizeof (elem_t));

    if (stack->hash_data != hash_data) {
        OPEN_DUMPFILE()
        fprintf (dumpfile, "From file %s function %s was called on %d line\n\n", file, function, line);
        fprintf (dumpfile, "Hash of data mismatch. Expected value: %d, Actual value: %d\n", stack->hash_data, hash_data);
        STACK_DUMP (stack);
        return ERROR_HASH;
    }

    const unsigned hash_stack_exp = stack->hash_stack;
    unsigned hash_stack_act = calc_stack_hash (stack);
    stack->hash_stack = hash_stack_exp;

    if (hash_stack_exp != hash_stack_act) {
        OPEN_DUMPFILE()
        fprintf (dumpfile, "from file %s function %s was called on %d line\n\n", file, function, line);
        fprintf (dumpfile, "Hash of stack mismatch. Expected value: %d, Actual value: %d\n", hash_stack_exp, hash_stack_act);
        STACK_DUMP (stack);
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
    fprintf (dumpfile, "Stack [%p]\n", stack);
    fprintf (dumpfile, "Current size of stack is: %zu\t\t\tcapacity is: %zu\n", stack->size, stack->capacity);

    fprintf (dumpfile, "Left  canary of data is: 0x%llx\n", stack->canary_data_left);
    fprintf (dumpfile, "Right canary of data is: 0x%llx\n", stack->canary_data_right);
    fprintf (dumpfile, "Left  canary of stack is: 0x%llx\n", stack->canary_st_left);
    fprintf (dumpfile, "Right canary of stack is: 0x%llx\n", stack->canary_st_right);
    fprintf (dumpfile, "Hash of stack's data is: %x\n", stack->hash_data);
    fprintf (dumpfile, "Hash of stack is: %x\n\n", stack->hash_stack);

    fprintf (dumpfile, "Elements of data are:\n");
    print_data_elem (stack);
    fprintf (dumpfile, "\n");
}

void print_data_elem (Stack *stack) {
    for (int i = 0; i < stack->capacity; i++) {
        if (i < stack->size) {
            fprintf(dumpfile, "*[%d] = %d\n", i, stack->data[i]);
        }
        else {
            fprintf (dumpfile, " [%d] = %x\n", i, stack->data[i]);
        }
    }
}

int stack_test (Stack *stack, const long capacity) {
    int status_ctor = stack_ctor (stack, capacity);

    assert (status_ctor == NO_ERROR);

    for (int i = 0; i < 40; i++) {
        int status_push = stack_push(stack, 2 * i);
        assert (status_push == NO_ERROR);
    }

    for (int i = 0; i < 30; i++) {
        stack_pop (stack);
    }

    OPEN_DUMPFILE()
    STACK_DUMP (stack);
    stack_dtor (stack);
}


int open_dumpfile () {
    static int number_open = 0;

    if (number_open == 0) {
        dumpfile = fopen("../dump.txt", "w");
        number_open++;

    } else if (number_open == 1) {
        fclose (dumpfile);
        dumpfile = fopen("../dump.txt", "a");
        number_open++;
    }

    if (dumpfile == nullptr) {
        perror ("Error in opening dumpfile\n");
        return ERROR_OPEN_FILE;
    }
}