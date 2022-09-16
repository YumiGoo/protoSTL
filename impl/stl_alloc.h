#pragma once

//一级分配器
template <int inst> //用不到的模板参数
class __malloc_alloc_template {

};

//次级分配器 目前没有考虑多线程情况
template <int inst> //用不到的模板参数
class __default_alloc_template {
    static void* allocate(size_t n) {

    }

    static void deallocate() {
        
    }
};