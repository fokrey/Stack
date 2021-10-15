#include "Stack.h"

int main () {
    Stack stack = {0};
    const long capacity = 20;

    stack_test (&stack, capacity);

    return 0;
}