/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Testing:
#include "Testing.h"

using detail::FunctionTraits;

template<typename tFunc>
void checkValue(tFunc)
{
    typename FunctionTraits<tFunc>::FirstArg arg = MyInt(42);
    (void)arg;
}

template<typename tFunc>
void checkUniref(tFunc &&)
{
    typename FunctionTraits<tFunc>::FirstArg arg = MyInt(42);
    (void)arg;
}

template<typename tFunc>
void checkConstRef(const tFunc &)
{
    typename FunctionTraits<tFunc>::FirstArg arg = MyInt(42);
    (void)arg;
}

template<typename tFunc>
void checkRef(tFunc &)
{
    typename FunctionTraits<tFunc>::FirstArg arg = MyInt(42);
    (void)arg;
}

#define CHECK_TRAIT_RVALUE(__func)                                             \
do {                                                                           \
    checkValue(__func);                                                        \
    checkUniref(__func);                                                       \
    checkConstRef(__func);                                                     \
} while ( !42 )

#define CHECK_TRAIT(__func)                                                    \
do {                                                                           \
    CHECK_TRAIT_RVALUE(__func);                                                \
    checkUniref(std::move(__func));                                            \
    checkRef(__func);                                                          \
} while ( !42 )

void myTraitFunc(MyInt) {}
class MyTraitClass
{
public:
    void func(MyInt) {}
    void constFunc(MyInt) const {}
};

TEST(FunctionTraitsTest, sanityCheck)
{
    // lambda
    auto lambda = [](MyInt) {};
    CHECK_TRAIT(lambda);

    // free function
    CHECK_TRAIT(myTraitFunc);
    CHECK_TRAIT_RVALUE(&myTraitFunc);

    // std::function
    std::function<void(MyInt)> func;
    CHECK_TRAIT(func);

    // class method
    CHECK_TRAIT_RVALUE(&MyTraitClass::func);
    CHECK_TRAIT_RVALUE(&MyTraitClass::constFunc);
}
