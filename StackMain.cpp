// #define ON_DEBUG
// #define ON_HASH
// #define ON_CANARY_PROTECTION
#include "stack.h"


int main()
{
    stack_t stk1 = {};
    STACK_INIT(stk1, 5);
    StackPush(&stk1, 2.8);
    StackPush(&stk1, 3.4);
    StackPush(&stk1, 6.8);

    StackPush(&stk1, 7.9);
    StackPush(&stk1, 7.9);
    StackPush(&stk1, 7.9);
    StackPush(&stk1, 7.9);
    
    StackDestroy(&stk1);

    stk1.size = 100;

    StackPush(&stk1, 7.9);

    *(int*)&stk1 = 0;
    //qsort(&stk1, sizeof(stk1), sizeof(char), &CompareUp);
    stk1.data[stk1.capacity + 1] = 0;


    StackPush(&stk1, 0.9);

    double result = 0;
    StackPop(&stk1, &result);
    printf("%lg", result);
    //printf("%s\n%s\n", __DATE__, __TIME__);

    return 0;
}

