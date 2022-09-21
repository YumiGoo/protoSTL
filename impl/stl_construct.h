#pragma once

#include <new.h> //包含placement new
#include <stl_algobase.h>
#include <iterator_traits.h>
#include <type_traits.h>


/* ************************ 构造器 ************************ */

template <typename T1, typename T2>
inline void construct(T1* p, const T2& value) {
    new (p) T1(value);//placement new
}


/* ************************ 析构器 ************************ */

//对具体特定对象的析构
template <typename T>
void destroy(T* t) {
    t->~T();
}

//对两个迭代器之间元素的析构，需要判断trival
template <typename Iterator>
void destroy(Iterator first, Iterator last) {
    __destroy(first, last, value_type(first));//调用iterator全局函数
}

template <typename Iterator, typename T>
void __destroy(Iterator first, Iterator last, T*) {
    typedef __type_traits<T>::has_trivial_destructor has_trivial_destructor;
    __destroy_aux(first, last, has_trivial_destructor());
}

template <typename Iterator>
void __destroy_aux(Iterator first, Iterator last, __true_type) {
    //什么都不做
}

//对需要调用析构函数的元素
template <typename Iterator>
void __destroy_aux(Iterator first, Iterator last, __false_type) {
    //依次调用析构函数
    for (;first != last; ++first) {
        destroy(&*first);//取指针
    }
}

//似乎没有必要
template <typename T>
void __destroy_aux(T* first, T* last, __false_type) {
    size_t n = last - first;
    for (int i = 0; i < n; ++i, ++first) {
        destroy(&*first);
    }
}

//对于字符数组无需析构
void destroy(char* first, char* last) {}
void destroy(wchar_t* first, wchar_t* last) {}