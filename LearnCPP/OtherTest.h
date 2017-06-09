//
//  OtherTest.h
//  LearnCPP
//
//  Created by javalong on 2017/6/9.
//  Copyright © 2017年 javalong. All rights reserved.
//

#ifndef OtherTest_h
#define OtherTest_h

#include <stdlib.h>

#ifndef TEST_VARIABLE_IS_NOT_USED
#ifdef __GNUC__
#define TEST_VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define TEST_VARIABLE_IS_NOT_USED
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
    __inline int test_atstartup(void (*func)(void)) { func(); return 0;}
    __inline int test_atexit(void (*func)(void)) { return atexit(func);}
    
#ifdef __cplusplus
}
#endif

#define TEST_STARTUP(func) TEST_VARIABLE_IS_NOT_USED static int __anonymous_startup_##func = test_atstartup(func)
#define TEST_EXIT(func)    TEST_VARIABLE_IS_NOT_USED static int __anonymous_exit_##func = test_atexit(func)

static void testOther()
{
    printf("===testOther===\n");
}

TEST_STARTUP(testOther);

#endif /* OtherTest_h */
