#pragma once

#include <stdlib.h>


//一级分配器 用于大开销的内存分配和内存溢出情况的处理
template <int inst /* 用不到的模板参数, 这个参数应该代表的是instance，估计可能是可以根据一些编译期常量来对应不同的实现，比如说特化一些debug版本或者别的什么 */> 
class __malloc_alloc_template {
private:
    //用来处理OOM情况
    static void* oom_malloc(size_t);
    static void* oom_realloc(size_t);
    static void (* __malloc_alloc_oom_handler)();//模仿C++的new_handler，由用户提供内存不足时的处理例程。
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
        ptr = realloc(ptr, n);
        if (!ptr) {
            ptr = oom_realloc(ptr, n);
        }
        return ptr;
    }
};

typedef __malloc_alloc_template<0> malloc_alloc;

//次级分配器 目前没有考虑多线程情况
template <int inst> //用不到的模板参数
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