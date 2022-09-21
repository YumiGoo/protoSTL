#pragma once

#include <iostream>

//单元测试模板
template <typename Equality, typename T>
class Unit_test {
private:
    static int main_ret;
    static int test_count;
    static int test_pass;
public:
    static void __expect_eq(Equality, T, T);

};


//单元测试框架
template <typename Equality, typename T>
void Unit_test<Equality, T>::__expect_eq(Equality equality, T expect, T actual) {
    ++test_count;
    if (equality(expect, actual)) {
        ++test_pass;
    } else {
        std::cerr << __FILE__ << ":"<< __LINE__ << ": except: " << expect << "actual: " << actual << endl; 
    }
}


// 初值设定

template <typename Equality, typename T> int Unit_test<Equality, T>::main_ret = 0;//测试返回值
template <typename Equality, typename T> int Unit_test<Equality, T>::test_count = 0;//总测试次数
template <typename Equality, typename T> int Unit_test<Equality, T>::test_pass = 0;//通过测试次数

// 比较函数

//朴素比较函数
template <typename T>
static bool plain_equal(const T& lhs, const T& rhs) {
    return lhs == rhs;
}