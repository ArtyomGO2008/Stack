#pragma once

#define ON_DEBUG
//#define ON_CANARY_PROTECTION
//#define ON_HASH

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <time.h>
#include <assert.h>




#ifdef ON_DEBUG
    #define STACK_DEBUG(...) __VA_ARGS__
    #define STACK_VERIFY(stk)                    \
            err = StackVerify(stk);              \
            if (err != 0)                        \
            {                                    \
                StackDump("log_stack", err, stk);\
                return err;                      \
            }

#else
    #define STACK_DEBUG(...)
    #define STACK_VERIFY(stk)
#endif

#ifdef ON_CANARY_PROTECTION
    #define CANARY_PROTECTION(...) __VA_ARGS__
#else
    #define CANARY_PROTECTION(...)
#endif

#ifdef ON_HASH
    #define STACK_HASH(...) __VA_ARGS__
#else
    #define STACK_HASH(...)
#endif

static const uint64_t DATA_LEFT_CANARY   = 0xDEADBEEFCAFEBABEULL;
static const uint64_t DATA_RIGHT_CANARY  = 0xFEEDFACEDEADBEEFULL;
static const uint64_t STACK_LEFT_CANARY  = 0xEB1AADC0DEFA11EDULL;
static const uint64_t STACK_RIGHT_CANARY = 0xDE1E7E17C0DEFA11ULL;


typedef double StackElem_t;

struct stack_t
{
    CANARY_PROTECTION (uint64_t stack_left_canary = STACK_LEFT_CANARY;)

    StackElem_t* data;
    int size;
    int capacity;

    STACK_DEBUG
    (
        const char* var_name;
        const char* file;
        const char* func;
        const char* logfile;
        int line;
    )

    STACK_HASH (uint64_t hash;)

    CANARY_PROTECTION (uint64_t stack_right_canary = STACK_RIGHT_CANARY;)
};


#ifdef ON_HASH
    void DumpHash (FILE* log_file, uint64_t errors);
    uint64_t StackCalculateHash (stack_t* stk);
    uint64_t djb2_hash(uint64_t hash, const void* ptr, size_t size);
    int StackUpdateHash(stack_t* stk);
    uint64_t CheckHash (stack_t* stk, uint64_t* errors);
#else
    static inline void DumpHash (FILE* log_file, uint64_t errors){}
    static inline uint64_t StackCalculateHash (stack_t* stk){}
    static inline uint64_t djb2_hash(uint64_t hash, const void* ptr, size_t size){}
    static inline int StackUpdateHash(stack_t* stk){}
    static inline uint64_t CheckHash (stack_t* stk, uint64_t* errors){}

#endif

#define CANARY_SIZE sizeof(uint64_t)

#ifdef ON_CANARY_PROTECTION

    static inline StackElem_t* StackAllocData(int capacity)
    {
        size_t total_bytes = capacity * sizeof(StackElem_t) + 2 * CANARY_SIZE;
        return (StackElem_t*)malloc(total_bytes);
    }

    static inline StackElem_t* StackReallocData(StackElem_t* old_data, int new_capacity)
    {
        size_t total_bytes = new_capacity * sizeof(StackElem_t) + 2 * CANARY_SIZE;
        return (StackElem_t*)realloc(old_data, total_bytes);
    }

    void SetDataCanaries(stack_t* stk, uint64_t DataLeftCanary, uint64_t DataRightCanary);
    void CheckStackCanaries (stack_t* stk, uint64_t* errors);
    void CheckDataCanaries (stack_t* stk, uint64_t* errors);
    void DumpCanaries (FILE* log_file, uint64_t errors);
    #define DATA(i) stk->data[i + 1]
#else

    static inline StackElem_t* StackAllocData(int capacity)
    {
        return (StackElem_t*)malloc(capacity * sizeof(StackElem_t));
    }

    static inline StackElem_t* StackReallocData(StackElem_t* old_data, int new_capacity)
    {
        return (StackElem_t*)realloc(old_data, new_capacity * sizeof(StackElem_t));
    }

    static inline void SetDataCanaries(stack_t* stk, uint64_t DataLeftCanary, uint64_t DataRightCanary){}
    static inline void CheckStackCanaries (stack_t* stk, uint64_t* errors){}
    static inline void CheckDataCanaries (stack_t* stk, uint64_t* errors){}
    static inline void DumpCanaries (FILE* log_file, uint64_t errors){}
    #define DATA(i) stk->data[i]
#endif


#define STACK_INIT(stk, capacity)\
        StackInit (&stk, capacity STACK_DEBUG(, __FILE__, #stk, __func__, __LINE__))


static uint64_t err = 0;

enum StackError_t : uint64_t
{
    STACK_OK           = 0x00,
    STACK_ERR_NULL     = 0x01, // &stack = NULL
    STACK_ERR_DATA     = 0x02, // &data = NULL
    STACK_ERR_CAPACITY = 0x04, // capacity <= 0
    STACK_ERR_SIZE     = 0x08,     // size < 0
    STACK_ERR_OVERFLOW = 0x10, // size > capacity
    STACK_ERR_REALLOC_FAILED = 0x20,
    STACK_ERR_DATA_LCANARY = 0x40,
    STACK_ERR_DATA_RCANARY = 0x80,
    STACK_ERR_STACK_LCANARY = 0x100,
    STACK_ERR_STACK_RCANARY = 0x200,
    STACK_ERR_HASH = 0x400
};


//Stack functions
stack_t* StackInit (stack_t* stk, int capacity
                    STACK_DEBUG(, const char* filename, const char* var_name, const char* func_name, int line));
uint64_t StackPush (stack_t* stk, StackElem_t value);
uint64_t StackPop (stack_t* stk, double* result);
uint64_t StackVErify(stack_t* stk);
stack_t* Reallocation (stack_t* stk);
uint64_t StackDestroy (stack_t* stk);

//PrintOUT
void StackDump (const char* log_filename, uint64_t errors, stack_t* stk);
void PrintData (FILE* log_file, stack_t* stk);
void PrintErrors (FILE* log_file, uint64_t errors);

//Hash funcs
uint64_t djb2_hash (uint64_t hash, const void *ptr, size_t size);
int  StackUpdateHash    (stack_t* stk);
uint64_t CheckHash (stack_t* stk, uint64_t* errors);
uint64_t StackCalculateHash (stack_t* stk);

//Canary protection
void SetDataCanaries(stack_t* stk, uint64_t DataLeftCanary, uint64_t DataRightCanary);
void CheckDataCanaries (stack_t* stk, uint64_t* errors);
void CheckStackCanaries(stack_t* stk, uint64_t* errors);


#pragma once
