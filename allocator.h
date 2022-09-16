#pragma once

//一个接口，供容器和客端调用
//在这里完成了size_t到字节数的转换
template <typename T, typename Alloc>
class simple_alloc {
public:
    static T* allocate(void) {
        return static_cast<T*>(Alloc::allocate(sizeof(T))); //这里改用C++类型转换
    }
    static T* allocate(size_t n) {
        if (0 == n) return 0;
        return static_cast<T*>(Alloc::allocate(n * sizeof(T))); //这里改用C++类型转换
    }
    static void deallocate(T* ptr) {
        Alloc::deallocate(ptr, sizeof(T));
    }
    static void deallocate(T* ptr, size_t n) {
        if (0 == n) return;
        Alloc::deallocate(ptr, n * sizeof(T));
    }
};
