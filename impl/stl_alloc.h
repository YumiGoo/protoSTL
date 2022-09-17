#pragma once

#include <stdlib.h>

#ifndef __THROW_BAD_ALLOC
#   include <iostream>
    //这里定义多条语句时需要防止发生歧义, 如果只用大括号可能会导致if意外结束使得else无法匹配
    //ex. if{}; else 大括号外的;导致else无法匹配if
#   define __THROW_BAD_ALLOC do{cerr << "out of memory" << endl; exit(0);}while(0) 
#endif

/* ************************ 初级分配器 ************************ */
//一级分配器 用于大开销的内存分配和内存溢出情况的处理
template <int inst /* 用不到的模板参数, 这个参数应该代表的是instance，估计可能是可以根据一些编译期常量来对应不同的实现，比如说特化一些debug版本或者别的什么 */> 
class __malloc_alloc_template {
private:
    //用来处理OOM情况
    static void* oom_malloc(size_t);
    static void* oom_realloc(void*, size_t);
    
    // static void (* __malloc_alloc_oom_handler)();
    typedef void (* __oom_handler)();//回调函数类型
    static __oom_handler __malloc_alloc_oom_handler;//模仿C++的new_handler，由用户提供的内存不足时的处理例程（本质回调函数）。

public:
    static void* allocate(size_t n) {
        void* ptr = malloc(n);
        if (!ptr) {
            //触发OOM机制
            ptr = oom_malloc(n);
        }
        return ptr;
    }

    static void deallocate(void* ptr, size_t) {
        free(ptr);//释放malloc分配的内存无需提供size
    }

    static void* reallocate(void* ptr, size_t /* old_n */, size_t n) {
        ptr = realloc(ptr, n);//直接调用realloc完事
        if (!ptr) {
            //触发OOM机制
            ptr = oom_realloc(ptr, n);
        }
        return ptr;
    }

    // //模仿自set_new_handler, 指定用户自己的处理例程
    // static void (*set_malloc_alloc_oom_handler)(void (*f()))() {
    //     void (*old_handler)() = __malloc_alloc_oom_handler;
    //     __malloc_alloc_oom_handler = f;
    //     return old_handler;//返回原本的handler
    // }
    
    //利用typedef提高可读性
    //模仿自set_new_handler, 指定用户自己的处理例程
    static __oom_handler set_malloc_alloc_oom_handler(__oom_handler f)() {
        __oom_handler old_handler = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = f;
        return old_handler;//返回原本的handler
    }
};

template <int inst>
void(* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void* __malloc_alloc_template<inst>::oom_malloc(size_t n) {
    void* ptr = 0;
    while (1) {
        if (__malloc_alloc_oom_handler == 0) __THROW_BAD_ALLOC;//这句为什么要在循环中？
        (*__malloc_alloc_oom_handler)();//不断调用处理例程释放内存
        ptr = malloc(n);
        if (ptr) return ptr;
    }
}

template <int inst>
void* __malloc_alloc_template<inst>::oom_realloc(void* ptr, size_t n) {
    while (1) {
        if (__malloc_alloc_oom_handler == 0) __THROW_BAD_ALLOC;//这句为什么要在循环中？
        (*__malloc_alloc_oom_handler)();//不断调用处理例程释放内存
        ptr = realloc(ptr, n);
        if (ptr) return ptr;
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;


/* ************************ 次级分配器 ************************ */
//次级分配器 目前没有考虑多线程情况
template <int inst>
class __default_alloc_template {
public:
    static void* allocate(size_t n) {
        if (n > 128) { // 调用初级分配器
            
        }

    }

    static void deallocate(void* ptr, size_t n) {
        if (n > 128) {
            
        }
    }
};