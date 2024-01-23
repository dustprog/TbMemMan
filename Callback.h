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
// Callback.h
//
// The callback methods for the Core memeory manager
//================================================================================================================
// History
//
// Created on 12/21/2023 by Dustin Watson
//================================================================================================================
#ifndef __TBMAN_CALLBACK_H
#define __TBMAN_CALLBACK_H
//================================================================================================================
//================================================================================================================

//
// Includes
//

#include "TbManUtility.h"

//================================================================================================================
//================================================================================================================
namespace Core {
    class TbMemMan;

    /// function pointer to generalized alloc function
    typedef void* (*fp_alloc)(void* current_ptr, size_t current_bytes, size_t requested_bytes, size_t* granted_bytes);
    typedef void (*void_callback1)(void* arg, void* ptr, size_t space);
    typedef void (*void_callback2)(void* arg, void* ptr);
    typedef bool (*bool_callback1)(void* arg, void* ptr, size_t space);
    typedef bool (*bool_callback2)(void* arg, void* ptr);
    typedef void* (*alloc_callback)(void* data, size_t size);

    typedef struct diagnostic { TbMemMan* man; void** ptr_arr; size_t* spc_arr; size_t size; } diagnostic;

    typedef struct mnode { void* p; size_t s; } mnode;
    typedef struct mnode_arr { mnode* data; size_t size; size_t space; } mnode_arr;

    typedef struct ext_for_instance_arg
    {
        void (*cb)(void* arg, void* ptr, size_t space);
        void* arg;
    } ext_for_instance_arg;

    static void ext_for_instance(void* arg, void* key, size_t val)
    {
        ext_for_instance_arg* iarg = (ext_for_instance_arg*)arg;
        iarg->cb(iarg->arg, key, val);
    }

    static void ext_count(void* arg, void* key, size_t val)
    {
        *(size_t*)arg += 1;
    }

    static /*inline*/ void forEachInstanceCollectCallback(void* arg, void* ptr, size_t space)
    {
        assert(arg);

        mnode_arr* arr = (mnode_arr*)arg;

        assert(arr->size < arr->space);

        //arr->data[ arr->size ] = ( mnode ){ .p = ptr, .s = space };

        mnode mnode;
        mnode.p = ptr;
        mnode.s = space;

        arr->data[arr->size] = mnode;
        arr->size++;
    }

    // inline should be used in the cpp file to define the method

    static /*inline*/ void* stdlib_alloc(void* current_ptr, size_t requested_size)
    {
        if (requested_size == 0)
        {
            if (current_ptr)
                free(current_ptr);
            
            current_ptr = NULL;
        }
        else
        {
            if (current_ptr)
            {
                void* ptr = realloc(current_ptr, requested_size);

                if (ptr == NULL)
                {
                    ERR("Memory reallocation failed, the program will terminate.");
                    //cout << "Memory reallocation failed, the program will terminate." << endl;
                    // free( this->digits ); provided that the destructor does not call free itself
                    exit(0);
                }

                current_ptr = ptr;
            }
            else
            {
                current_ptr = malloc(requested_size);
            }
            
            if (!current_ptr)
                ERR("Failed allocating %zu bytes", requested_size);
        }

        return current_ptr;
    }

    // generalized alloc function purely based on stdlib
    static /*inline*/ void* external_alloc(void* current_ptr, size_t requested_bytes, size_t* granted_bytes)
    {
        if (requested_bytes == 0)
        {
            if (current_ptr)
                free(current_ptr);
            
            current_ptr = NULL;
            
            if (granted_bytes)
                *granted_bytes = 0;
        }
        else
        {
            if (current_ptr)
            {
                void* ptr = realloc(current_ptr, requested_bytes);

                if (ptr == NULL)
                {
                    ERR("Memory reallocation failed, the program will terminate.");
                    //cout << "Memory reallocation failed, the program will terminate." << endl;
                    // free( this->digits ); provided that the destructor does not call free itself
                    exit(0);
                }

                current_ptr = ptr;
            }
            else
            {
                current_ptr = malloc(requested_bytes);
            }
            
            if (!current_ptr)
            {
                fprintf(stderr, "Failed allocating %zu bytes", requested_bytes);
                abort();
            }
            
            if(granted_bytes)
                *granted_bytes = requested_bytes;
        }
        
        return current_ptr;
    }

    // generalized alloc function purely based on stdlib
    static /*inline*/ void* external_nalloc(void* current_ptr, size_t current_bytes, size_t requested_bytes, size_t* granted_bytes)
    {
        return external_alloc(current_ptr, requested_bytes, granted_bytes);
    }
}
//================================================================================================================
//================================================================================================================
#endif//__TBMAN_CALLBACK_H