#include "mini_test.hpp"
#include <array>
#include <stdlib.h>

#define TGC_CUSTOM_HASH
size_t tgc_hash(void *ptr) {
    const uintptr_t ad = (uintptr_t) ptr;
    return (size_t) (ad >> 12);
}

#include "../tgc.h"
#include "../tgc.c"

const int entity_count = 1024;
const int test_count = 300;

struct p64_struct {
    p64_struct* values[8] = {nullptr};
};

tgc_t gc;
std::array<p64_struct*,entity_count> *ptrs = nullptr;
int balance = 0;
int balance_expected = 0;


// initialize test
TEST(test0)
{
    tgc_start(&gc, ptrs->end());

    for (int i = 0; i < entity_count; i++) {
        ptrs->at(i) = (p64_struct*)tgc_alloc_opt(&gc, sizeof(p64_struct), 0, [](void*){balance--;});
        EXPECT_NE(ptrs->at(i),(void*)0x0);
        new (ptrs->at(i)) p64_struct();
        balance++;
        balance_expected++;
    }
    tgc_run(&gc);
    EXPECT_EQ(balance,balance_expected);
    EXPECT_EQ(balance,entity_count);
    EXPECT_EQ(gc.nitems,entity_count);
}

// add pointer to pointer
TEST(test1)
{
    for (int i = 0; i < test_count;i++){
        long rng = random() % entity_count;
        if(ptrs->at(rng)->values[0] == nullptr)
        {
            ptrs->at(rng)->values[0] = (p64_struct*) tgc_alloc_opt(&gc, sizeof(p64_struct), 0, [](void*){balance--;});
            EXPECT_NE(ptrs->at(rng)->values[0],(void*)0x0);
            new (ptrs->at(rng)->values[0]) p64_struct();
            balance++;
            balance_expected++;
        }
    }
    tgc_run(&gc);
    EXPECT_EQ(balance,balance_expected);
    EXPECT_EQ(gc.nitems,balance_expected);
}

// duplicate (must be no change)
TEST(test2)
{
    for (int i = 0; i < test_count; i++){
        long rng = random() % entity_count;
        tgc_add_ptr(&gc, ptrs->at(rng), sizeof(p64_struct), 0, [](void*){balance--;});
    }
    tgc_run(&gc);
    EXPECT_EQ(balance,balance_expected);
    EXPECT_EQ(gc.nitems,balance_expected);
}


// delete or pass (first Half)
TEST(test3)
{
    for (int i = 0; i < entity_count/2; i++)
        if(random()&0x7){
            if(ptrs->at(i)->values[0])
                balance_expected--;
            tgc_free(&gc, ptrs->at(i));
            balance_expected--;
        }
    tgc_run(&gc);
    EXPECT_EQ(balance,balance_expected);
    EXPECT_EQ(gc.nitems,balance_expected);
}


// leave or pass (second Half)
TEST(test4)
{
    for (int i = (entity_count/2+1); i < entity_count; i++)
        if(random()&0x7){
            if(ptrs->at(i)->values[0])
                balance_expected--;
            ptrs->at(i) = nullptr;
            balance_expected--;
        }
    tgc_run(&gc);
    EXPECT_EQ(balance,balance_expected);
    EXPECT_EQ(gc.nitems,balance_expected);
}



// finalize
TEST(test5)
{
    memset(ptrs->data(),0,sizeof(void*) * ptrs->size());
    tgc_run(&gc);
    EXPECT_EQ(balance,0);
    EXPECT_EQ(gc.nitems,0);
}

// destroy
TEST(test6)
{
    tgc_stop(&gc);
}


int main(int argc, char **argv) {
    std::array<p64_struct*,entity_count> stack_memory;
    ptrs = &stack_memory;
    srandom(time(NULL));
    mtest::run_all();
    return 0;
}