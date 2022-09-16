#pragma once

#include <cstddef>
#include "iterator.h"

//迭代器类型萃取器
template <typename Iterator>
struct iterator_traits {
    typedef Iterator::iterator_category iterator_category;
    typedef Iterator::value_type value_type;
    typedef Iterator::difference_type difference_type;
    typedef Iterator::pointer pointer;
    typedef Iterator::reference reference;
};

//指针特化版本
template <typename T>
struct iterator_traits<T*> {
    typedef random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;
};

template <typename T>
struct iterator_traits<const T*> { //萃取出的类型不应带有const属性
    typedef random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;
};

//包装萃取器的工具函数
template <typename Iterator>
inline typename iterator_traits<Iterator>::iterator_category iterator_category(const Iterator&) {
    return iterator_traits<Iterator>::iterator_category();
}

template <typename Iterator>
inline typename iterator_traits<Iterator>::value_type* value_type(const Iterator&) {
    return static_cast<iterator_traits<Iterator>::value_type*>(0); //这里返回的是指向类型的指针，因为要初始化，返回指针能够节省初始化开销
}

template <typename Iterator>
inline typename iterator_traits<Iterator>::difference_type* difference_type(const Iterator&) {
    return static_cast<iterator_traits::<Iterator>::difference_type*>(0);
}

// template <typename Iterator>
// inline typename iterator_traits<Iterator>::pointer pointer(const Iterator&) {
//     return iterator_traits<Iterator>::pointer();
// }