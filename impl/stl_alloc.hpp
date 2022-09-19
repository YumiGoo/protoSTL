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

    //模仿自set_new_handler, 指定用户自己的处理例程
    // static void (*set_malloc_alloc_oom_handler)(void (*f()))() {
    //     void (*old_handler)() = __malloc_alloc_oom_handler;
    //     __malloc_alloc_oom_handler = f;
    //     return old_handler;//返回原本的handler
    // }
    
    //利用typedef提高可读性
    //模仿自set_new_handler, 指定用户自己的处理例程
    static __oom_handler set_malloc_alloc_oom_handler(__oom_handler f) {
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

typedef __malloc_alloc_template<0> malloc_alloc;//模板特化，一级分配器


/* ************************ 次级分配器 ************************ */
enum {__ALIGN = 8};//freelist的最小单位
enum {__MAX_BYTES = 128};//freelist节点的最大size
enum {__NFREELISTS = __MAX_BYTES / __ALIGN};//freelist数量

//次级分配器 目前没有考虑多线程情况
template <int inst>
class __default_alloc_template {
public:
    static void* allocate(size_t n) {
        // 调用初级分配器
        if (n > (size_t)__MAX_BYTES) {//统一有符号数和无符号数
            return malloc_alloc::allocate(n);
        }
        FreeListNode* volatile* my_list;//这里声明时考虑到了多线程
        my_list = free_list + calculate_list_index(n);
        FreeListNode* ptr = *my_list;
        //如果该尺寸的链表空了，调用refiil来重新填充链表
        if (ptr == 0) {
            ptr = refill(n);
            return ptr;
        }
        *my_list = ptr->next;
        return ptr;
    }

    static void deallocate(void* ptr, size_t n) {
        if (n > (size_t)__MAX_BYTES) {
            malloc_alloc::deallocate(ptr, n);
            return;
        }
        FreeListNode* list = free_list[calculate_list_index(n)];
        FreeListNode* p = (FreeListNode*)ptr;
        p->next = list;
        list = p;
    }

    static void* reallocate(void* ptr, size_t old_n, size_t new_n);
private:
    //工具函数

    //计算应分配哪个链表中的链表节点
    static inline size_t calculate_list_index(size_t n) {
        return (n + __ALIGN - 1) / __ALIGN - 1;
    }

    //将分配的内存尺寸规约为__ALIGN的倍数
    static inline size_t round_up_to_align(size_t n) {
        // return (n + __ALIGN - 1) / __ALIGN * __ALIGN;
        return (n + __ALIGN - 1) & ~(__ALIGN - 1); //因为__ALIGN一定是8的倍数
    }
private:
    //数据成员

    union FreeListNode {//至少有8字节，可以存放指针，在分配前指针使用的空间和分配的空间在同一位置，因此不会有额外空间开销。
        union FreeListNode* next;
        char data[1];
    };

    //自由链表，用来管理低于__MAX_BYTES尺寸的空间
    static FreeListNode* volatile free_list[__NFREELISTS];
    
    //free_list填充
    static void* refill(size_t n);
    //内存池管理
    static char* chunk_alloc(size_t n, int& nFreeListNodes);

    //由chunk_alloc()管理
    static char* start_free;//内存池起始
    static char* end_free;//内存池结束
    static size_t heap_size;//在heap中占用的总空间（内存池+自由链表）
};

template <int inst> 
void* __default_alloc_template<inst>::refill(size_t n) {

}

template <int inst>
char* __default_alloc_template<inst>::chunk_alloc(size_t n, int& nFreeListNodes) {

}




typedef __default_alloc_template<0> default_alloc;//模板特化，次级分配器