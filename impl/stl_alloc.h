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

//初值

// template <int inst>
// void(* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst> __malloc_alloc_template<inst>::__oom_handler __malloc_alloc_oom_handler = 0;

typedef __malloc_alloc_template<0> malloc_alloc;//模板特化，一级分配器

#ifdef __USE_MALLOC
typedef malloc_alloc alloc;
#else


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
        //每次分配的size必然是__ALIGN的倍数
        my_list = free_list + calculate_list_index(n);
        FreeListNode* ptr = *my_list;
        //如果该尺寸的链表空了，调用refiil来重新填充链表
        if (ptr == 0) {
            //在这里进行了向上取__ALIGN的倍数，多分配的空间会在deallocate的时候回归链表
            ptr = refill(round_up_to_align(n));
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
        //每次回收的size必定是__ALIGN的倍数
        FreeListNode* volatile* my_list = free_list + calculate_list_index(n);
        
        FreeListNode* p = (FreeListNode*)ptr;
        p->next = *my_list;
        *my_list = p;
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

    //至少有8字节，可以存放指针，在分配前指针使用的空间和分配的空间在同一位置，因此不会有额外空间开销。
    union FreeListNode {
        union FreeListNode* next;
        char data[1];
    };

    //自由链表，用来管理低于__MAX_BYTES尺寸的空间，由refill、allocate、deallocate、reallocate共同使用和管理
    static FreeListNode* volatile free_list[__NFREELISTS];
    
    //free_list填充，接受的n必须是__ALIGN的倍数
    static void* refill(size_t n);
    //内存池管理
    static char* chunk_alloc(size_t n, int& nFreeListNodes);

    //由chunk_alloc()管理
    static char* start_free;//内存池起始
    static char* end_free;//内存池结束
    static size_t heap_size;//在heap中占用的总空间（内存池+自由链表）
};

//先不管 需要allocate新的空间，将原空间内的变量转移到新空间，并将原空间deallocate //未完成！！
template <int inst>
void* __default_alloc_template<inst>::reallocate(void* ptr, size_t old_n, size_t new_n) {
    return malloc_alloc::reallocate(ptr, old_n, new_n);
    //先将之前的空间归还，再尝试分配新空间
}

template <int inst> 
void* __default_alloc_template<inst>::refill(size_t n) {
    int nFreeListNodes = 20;//要新增的节点数
    //返回一个指向空chunk的指针，将此chunk的空间返回及构造新node
    char* ptr = chunk_alloc(n, nFreeListNodes);//这里传递引用，如果不足以分配默认数量的节点则会通过该值调整
    //如果只申请到一个节点，则直接将该节点返回客端
    if (nFreeListNodes == 1) return (FreeListNode*)ptr;
    //将多申请的节点加入自由链表
    FreeList* volatile* my_list = free_list + calculate_list_index(n);
    while (nFreeListNodes-- > 1) {
        FreeListNode* p = (FreeListNode*)ptr;
        ptr += n;
        p->next = *my_list;
        *my_list = p;
    }
    return (FreeListNode*)ptr;
}

//内存池管理的实现，最复杂的部分
template <int inst>
char* __default_alloc_template<inst>::chunk_alloc(size_t n, int& nFreeListNodes) {
    size_t chunk_size = end_free - start_free;
    size_t intend_size = n * nFreeListNodes;
    //A如果内存池中剩余空间足够分配所有节点，分配所有节点并返回
    if (chunk_size >= intend_size) {
        char* ptr = start_free;
        start_free += intend_size;
        return ptr;
    }

    //B如果内存池中剩余空间仍然可以分配至少一个节点，那就分配这些节点并返回
    if (chunk_size >= n) {
        nFreeListNodes = chunk_size / n;//向下取整 nFreeListNodes只会在此处更新
        char* ptr = start_free;
        start_free += intend_size;
        return ptr;
    }

    //C如果内存池剩余空间分配不了哪怕一个节点
    //C1首先将内存池中剩下的空间完全分配干净
    if (chunk_size > 0) {
        //最低也会剩下__ALIGN以上，因为总空间是__ALIGN的倍数，每次分配出去的也是__ALIGN的倍数
        FreeListNode* volatile* my_list = free_list + calculate_list_index(chunk_size);
        // FreeListNode* node = (FreeListNode*)start_free;//没有必要
        (FreeListNode*) start_free->next = *my_list;
        *my_list = (FreeListNode*) start_free;
    }
    //C2尝试从系统堆中分配新的空间作为新的内存池
    //每次新申请的内存空间是需求的两倍，并且加上一个随已用空间增长的附加量，两部分都是__ALIGN的倍数
    size_t new_size = intend_size * 2 + round_up_to_align(heap_size >> 4);//round_up(heap_size / 16)
    start_free = (char*)malloc(new_size);
    //C2A完全没有空间
    if (start_free == 0) {
        //C2AA尝试利用剩余freelist中的空节点
        for (size_t nn = n; nn <= __MAX_BYTES ;nn += __ALIGN) {
            FreeListNode* volatile* my_list = free_list + calculate_list_index(nn);
            if (*my_list != 0) {//将会释放空节点到内存池
                start_free = (char*)*my_list;
                *my_list = (*my_list)->next;
                end_free = start_free + nn;
                return chunk_alloc(n, nFreeListNodes);//递归调用自身，更新nFreeListNodes
            }
        }
        //C2AB此时仍然没有解决问题，触发OOM机制 （山穷水尽）
        end_free = 0;
        start_free = (char*)malloc_alloc::allocate(new_size);//调用一级分配器，尝试用OOM例程解决问题

    }

    //C2B分配到了新空间，递归调用此函数完成分配
    end_free = start_free + new_size;
    heap_size += new_size;
    return chunk_alloc(n, nFreeListNodes);//重新分配空间后递归调用
}

//初值

template <int inst> char* __default_alloc_template<inst>::start_free = 0;//内存池起始
template <int inst> char* __default_alloc_template<inst>::end_free = 0;//内存池结束
template <int inst> size_t __default_alloc_template<inst>::heap_size = 0;//在heap中占用的总空间（内存池+自由链表）
template <int inst> __default_alloc_template<inst>::FreeListNode* volatile free_list[__NFREELISTS] = 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};//自由链表，用来管理低于__MAX_BYTES尺寸的空间

typedef __default_alloc_template<0> default_alloc;//模板特化，次级分配器

typedef default_alloc alloc;

#endif
