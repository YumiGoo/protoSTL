#pragma once

#include <cstddef>

//五类迭代器
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

//迭代器原型
template <typename Category, typename T, typename Distance = std::ptrdiff_t, typename Pointer = T*, typename Reference = T&>
struct iterator { //所有迭代器都应该继承该结构体，保证不会缺少用于type_traits的内嵌型别定义
    //这里只有类型定义，不会有额外开销
    typedef Category iterator_category;
    typedef T value_type; 
    typedef Distance difference_type;
    typedef Pointer pointer;
    typedef Reference reference;
};

