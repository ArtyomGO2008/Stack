#include "stack.h"

#ifdef ON_CANARY_PROTECTION

void CheckStackCanaries (stack_t* stk, uint64_t* errors)
{
    if (stk->stack_left_canary != STACK_LEFT_CANARY)
        *errors |= STACK_ERR_STACK_LCANARY;

    if (stk->stack_right_canary != STACK_RIGHT_CANARY)
        *errors |= STACK_ERR_STACK_RCANARY;
}

void CheckDataCanaries (stack_t* stk, uint64_t* errors)
{
    if  (*(uint64_t*)&stk->data[0] != DATA_LEFT_CANARY)
        *errors |= STACK_ERR_DATA_LCANARY;

    if  (*(uint64_t*)&stk->data[stk->capacity + 1] != DATA_RIGHT_CANARY)
        *errors |= STACK_ERR_DATA_RCANARY;
}

void DumpCanaries (FILE* log_file, uint64_t errors)
{
    if (errors & STACK_ERR_DATA_LCANARY)
        fprintf(log_file, "[ERROR] Data left canary was damaged!\n");
    if (errors & STACK_ERR_DATA_RCANARY)
        fprintf(log_file, "[ERROR] Data right canary was damaged!\n");
    if (errors & STACK_ERR_STACK_LCANARY)
        fprintf(log_file, "[ERROR] Stack left canary was damaged!\n");
    if (errors & STACK_ERR_STACK_RCANARY)
        fprintf(log_file, "[ERROR] Stack right canary was damaged!1\n");
}


void SetDataCanaries(stack_t* stk, uint64_t DataLeftCanary, uint64_t DataRightCanary)
{
    *(uint64_t*)&stk->data[0] = DataLeftCanary;
    *(uint64_t*)&stk->data[stk->capacity + 1] = DataRightCanary;
}

#endif

#ifdef ON_HASH

uint64_t StackCalculateHash (stack_t* stk)
{
    if (!stk) return 0;

    uint64_t old_hash = stk->hash;
    stk->hash = 0;

    uint64_t current_hash = 5381;
    current_hash = djb2_hash(current_hash, stk, sizeof(stack_t));

    if (stk->data != NULL)
    {
        #ifdef ON_CANARY_PROTECTION
            size_t buffer_size = (stk->capacity + 2) * sizeof(double);
        #else
            size_t buffer_size = (stk->capacity) * sizeof(double);
        #endif
        current_hash = djb2_hash(current_hash, stk->data, buffer_size);
    }

    stk->hash = old_hash;

    return current_hash;
}

uint64_t djb2_hash(uint64_t hash, const void* ptr, size_t size)
{
    const unsigned char* byte_ptr = (const unsigned char*) ptr;

    for (size_t i = 0; i < size; i++)
        hash = ((hash << 5) + hash) + byte_ptr[i];

    return hash;
}

int StackUpdateHash(stack_t* stk)
{
    if (!stk) return 0;
    stk->hash = StackCalculateHash(stk);
}

void DumpHash (FILE* log_file, uint64_t errors)
{
    if (errors & STACK_ERR_HASH)
        fprintf(log_file, "[ERROR] Hash was changed!\n");
}

uint64_t CheckHash (stack_t* stk, uint64_t* errors)
{
    if (stk->hash != StackCalculateHash(stk))
        *errors |= STACK_ERR_HASH;

    return STACK_ERR_HASH;
}

#endif
