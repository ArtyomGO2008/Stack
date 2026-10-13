#include "stack.h"

#define POISON 6677.52
//TODO strerr, func set canary, typedef ull - err, struct stack_t calloc, stack destroy, условная компиляция функций хэша



stack_t* StackInit (stack_t* stk, int capacity STACK_DEBUG(, const char* filename, const char* var_name, const char* func_name, int line))
{
    assert(stk != NULL);

    stk->data = StackAllocData(capacity);

    assert (stk->data != NULL);

    stk->size = 0;
    stk->capacity = capacity;

    for (int i = 0; i < stk->capacity; i++)
        DATA(i) = POISON;

    SetDataCanaries(stk, DATA_LEFT_CANARY, DATA_RIGHT_CANARY);

    STACK_DEBUG
    (
        stk->var_name = var_name; // name of var
        stk->file = filename;
        stk->line = line;
        stk->func = func_name;
    )

    STACK_HASH (stk->hash = StackCalculateHash(stk);)

    //printf("size = <%d>, capacity = <%d>, filename = <%s>, var_name = <%s>, func_name = <%s>, line = <%d>\n",
            //stk->size, stk->capacity, stk->file, stk->var_name, stk->func, stk->line);
    return stk;
}

uint64_t StackVerify (stack_t* stk)
{
    uint64_t errors = STACK_OK;

    if (stk == NULL)
        return STACK_ERR_NULL;

    CheckHash (stk, &errors);

    if (stk->capacity <= 0)
        errors |= STACK_ERR_CAPACITY;

    if (stk->size < 0)
        errors |= STACK_ERR_SIZE;

    if (stk->size > stk->capacity)
        errors |= STACK_ERR_OVERFLOW;

    CheckStackCanaries(stk, &errors);

    if (stk->data == NULL)
        return errors |= STACK_ERR_DATA;

    CheckDataCanaries(stk, &errors);

    return errors;
}

uint64_t StackPush (stack_t* stk, StackElem_t value)
{
    STACK_VERIFY(stk)
    printf("after first verification in StackPush!\n");

    if (stk->size + 1 == stk->capacity)
    {
        Reallocation (stk);
        SetDataCanaries(stk, DATA_LEFT_CANARY, DATA_RIGHT_CANARY);
    }

    DATA(stk->size++) = value;

    StackUpdateHash(stk);

    //printf("left canary = %llX\n", stk->data[0]);

    STACK_VERIFY(stk)
    printf("After second verification in push!\n");
}

uint64_t StackPop (stack_t* stk, double* result)
{
    STACK_VERIFY(stk)

    StackElem_t value = DATA(stk->size);
    DATA(stk->size) = POISON;
    stk->size--;

    StackUpdateHash(stk);

    STACK_VERIFY(stk)

    *result = value;
}

uint64_t StackDestroy (stack_t* stk)
{
    STACK_VERIFY(stk)

    free (stk->data);
    stk->data = 0;
    stk->size = 0;
    stk->capacity = 0;

    #ifdef ON_CANARY_PROTECTION
        stk->stack_left_canary = 0;
        stk->stack_right_canary = 0;
    #endif

    #ifdef ON_HASH
        stk->hash = 0;
    #endif

    StackDump ("log_stack", 0, stk);
}


void StackDump (const char* log_filename, uint64_t errors, stack_t* stk)
{
    FILE* log_file = fopen(log_filename, "a");

    if (log_file == NULL)
    {
        printf("The file could not be opened\n");
        exit(EXIT_FAILURE);
    }

    char time_str[20] = {};
    time_t SistemTime = time(NULL);
    struct tm* timeinfo = localtime(&SistemTime);
    strftime(time_str, sizeof(time_str), "%d-%m-%Y %H:%M:%S", timeinfo);

    fprintf(log_file, "--- STACK DUMP [%s] ---\n", time_str);
    fprintf(log_file, "Stack_t \"%s\" [%p] created by [%s] at [%s : %d]\n",
            stk->var_name, stk, stk->func, stk->file, stk->line);

    PrintErrors(log_file, errors);

    if (stk != NULL)
    {
        fprintf(log_file, "{\n");

        CANARY_PROTECTION (fprintf(log_file, "\tStack left canary = %llX\n", stk->stack_left_canary);)

        fprintf(log_file, "\tcapacity = %d\n"
                          "\tsize     = %d\n"
                          "\tdata[%p]\n"
                          "\t{\n", stk->capacity, stk->size, stk->data);

        int i = 0;
        if (stk->data != NULL && stk->capacity > 0)
        {
            PrintData(log_file, stk);
        }

        fprintf(log_file, "\t}\n");

        STACK_HASH(fprintf(log_file, "\tExpected hash = %llu\n\tCurrent hash = %llu\n", stk->hash, StackCalculateHash(stk));)

        CANARY_PROTECTION(fprintf(log_file, "\tStack right canary = %llX\n", stk->stack_right_canary);)

        fprintf(log_file, "}\n");
    }
    else
        fprintf(log_file, "{\n\t[ERROR] Stack pointer is NULL\n}\n");

    fprintf(log_file,"----------------------------------------\n\n");

    fclose(log_file);
}


stack_t* Reallocation (stack_t* stk)
{
    int new_capacity = (stk->capacity) * 2;
    StackElem_t* temp = StackReallocData(stk->data, new_capacity) ;

    if (temp == NULL)
    {
        STACK_DEBUG(StackDump("log_stack", STACK_ERR_REALLOC_FAILED, stk));
        exit(EXIT_FAILURE);
    }

    stk->data = temp;

    //stk->data[stk->capacity + 1] = POISON;

    for (int i = stk->capacity; i < new_capacity; i++)
        DATA(i) = POISON;

    stk->capacity = new_capacity;
}

void PrintData (FILE* log_file, stack_t* stk)
{
    CANARY_PROTECTION (fprintf(log_file, "\t\tData left  canary = %llX\n", stk->data[0]);)

    for (int i = 0; i < stk->capacity; i++ )
    {
        if (DATA(i) != POISON)
            fprintf(log_file, "\t\t*[%d] = %lg\n", i, DATA(i));
        else
            fprintf(log_file, "\t\t [%d] = %lg (POISON)\n", i, DATA(i));
    }

    CANARY_PROTECTION (fprintf(log_file, "\t\tData right canary = %llX\n", stk->data[stk->capacity + 1]);)
}

void PrintErrors (FILE* log_file, uint64_t errors)
{
    fprintf(log_file, "ERROR CODE: 0x%016llX\n", errors);

    if (errors & STACK_ERR_NULL)
        fprintf(log_file, "[ERROR] Stack pointer is NULL!\n");
    if (errors & STACK_ERR_DATA)
        fprintf(log_file, "[ERROR] Data pointer is NULL!\n");
    if (errors & STACK_ERR_CAPACITY)
        fprintf(log_file, "[ERROR] Capacity is negative or zero!\n");
    if (errors & STACK_ERR_SIZE)
        fprintf(log_file, "[ERROR] Size is negative!\n");
    if (errors & STACK_ERR_OVERFLOW)
        fprintf(log_file, "[ERROR] Size > Capacity (Overflow)!\n");
    if (errors & STACK_ERR_REALLOC_FAILED)
        fprintf(log_file, "[ERROR] Reallocation failed!\n");

    DumpCanaries(log_file, errors);

    DumpHash (log_file, errors);
}


