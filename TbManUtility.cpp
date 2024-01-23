#include "TbManUtility.h"

//================================================================================================================
//================================================================================================================

void Core::TbManUtility::eval_wrnv(const char* format, va_list args)
{
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

//================================================================================================================
//================================================================================================================

void Core::TbManUtility::eval_err(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    eval_wrnv(format, args);
    va_end(args);
    abort();
}

//================================================================================================================
//================================================================================================================

uint32_t Core::TbManUtility::xsg_u2(uint32_t rval)
{
    rval ^= (rval >>  7);
    rval ^= (rval << 25);
    return rval ^ (rval >> 12);
}

//================================================================================================================
//================================================================================================================

void Core::TbManUtility::ext_err(const char* func, const char* file, int line, const char* format, ...)
{
    fprintf(stderr, "error in function %s (%s:%i):\n", func, file, line);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    abort();
}

//================================================================================================================
//================================================================================================================

void* Core::TbManUtility::aligned_alloc(size_t size, size_t alignment)
{
    if (alignment < alignof(void*))
    {
        alignment = alignof(void*);
    }
    
    size_t space = size + alignment - 1;
    void* allocated_mem = ::operator new(space + sizeof(void*));
    void* aligned_mem = static_cast<void*>(static_cast<char*>(allocated_mem) + sizeof(void*));
    
    ////////////// #1 ///////////////
    std::align(alignment, size, aligned_mem, space);
    
    ////////////// #2 ///////////////
    *(static_cast<void**>(aligned_mem) - 1) = allocated_mem;
    
    ////////////// #3 ///////////////
    return aligned_mem;
}

//================================================================================================================
//================================================================================================================

void Core::TbManUtility::aligned_free(void* p) noexcept
{
    ::operator delete(*(static_cast<void**>(p) - 1));
}

//================================================================================================================
//================================================================================================================