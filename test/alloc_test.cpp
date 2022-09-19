#include <stl_alloc.hpp>
#include <allocator.h>

int main() {
    simple_alloc<double, malloc_alloc> dataAllocator0;
    simple_alloc<double, default_alloc> dataAllocator1;
    simple_alloc<double, alloc> dataAllocator2;

    double *pd1 = dataAllocator0.allocate(1);
    
}