/**
Author & Copyright (C) 2017 Johannes Bernhard Steffens.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//================================================================================================================
// TbManUtility.h
//
// The utility methods for the Core memeory manager
//================================================================================================================
// History
//
// Created on 12/21/2023 by Dustin Watson
//================================================================================================================
#ifndef __TBMANUTILITY_H
#define __TBMANUTILITY_H
//================================================================================================================
//================================================================================================================

//
// Includes
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <memory>

//================================================================================================================
//================================================================================================================
namespace Core {
    class TbManUtility
    {
    public:

        /*template <class T>
        static void reset(T& t)
        {
            t = T();
        }*/

        static void eval_wrnv(const char* format, va_list args);

        static void eval_err(const char* format, ...);

        /** Xor Shift Generator.
         *  The random generator below belongs to the family of xorshift generators
         *  discovered by George Marsaglia (http://www.jstatsoft.org/v08/i14/paper).
         *  At approximately 50% higher CPU effort, these generators exhibit
         *  significantly better randomness than typical linear congruential
         *  generators.
         *
         *  (Not suitable for cryptographic purposes)
         */
        static uint32_t xsg_u2(uint32_t rval);

        static void ext_err(const char* func, const char* file, int line, const char* format, ...);

        static void* aligned_alloc(size_t size, size_t alignment);

        static void aligned_free(void* p) noexcept;
    };

#define ERR(...) TbManUtility::ext_err(__func__, __FILE__, __LINE__, __VA_ARGS__)

#define ASSERT_GLOBAL_INITIALIZED(manager) \
        if (manager == NULL) \
            ERR("Manager was not initialized. Call tbman_open() at the beginning of your program.")

    /// same purpose as assert() but cannot be switched off via NDEBUG; typically used in selftests
#define ASSERT(condition) \
        if (!(condition)) \
            TbManUtility::eval_err( \
                "assertion '%s' failed in function %s (%s line %i)\n", \
                #condition, \
                __func__, \
                __FILE__, \
                __LINE__)
}

//================================================================================================================
//================================================================================================================
#endif//__TBMANUTILITY_H