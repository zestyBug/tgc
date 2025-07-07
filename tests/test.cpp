#include "mini_test.hpp"
#define TGC_CUSTOM_HASH
size_t tgc_hash(void *ptr) {
    const uintptr_t ad = (uintptr_t) ptr;
    return (size_t) (ad & 2);
}
#include "../tgc.h"
#include "../tgc.c"

tgc_t gc;
const int test_count = 8;
void **ptrs = nullptr;
int counter;

// minimum test
TEST(test1){
    counter = 0;
    for (int i = 0; i < test_count; i++)
    {
        ptrs[i]=tgc_alloc_opt(&gc, 64, 0, [](void*){counter++;});
        EXPECT_NE(ptrs[i],(void*)0x0);
    }
    tgc_run(&gc);
    EXPECT_EQ(counter,0);
    EXPECT_EQ(gc.nitems,test_count);
}

// duplication test
TEST(test2){
    counter = 0;
    // manual injection:
    for (int i = 0; i < test_count; i++)
        tgc_add_ptr(&gc, ptrs[i], 64, 0, [](void*){counter++;});
    EXPECT_EQ(counter,0);
    EXPECT_EQ(gc.nitems,test_count);
}

// delete test
TEST(test3){
    memset(ptrs,0,sizeof(void*)*test_count);
    tgc_run(&gc);
    EXPECT_EQ(counter,test_count);
    EXPECT_EQ(gc.nitems,0);
}

int main(int argc, char **argv) {
    char *stack_memory[test_count];
    ptrs = (void**)stack_memory;
    tgc_start(&gc, (void*)(stack_memory + sizeof(stack_memory)));
    mtest::run_all();
    //tgc_stop(&gc);
    return 0;
}