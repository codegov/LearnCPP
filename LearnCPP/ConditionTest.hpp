//
//  ConditionTest.hpp
//  LearnCPP
//
//  Created by javalong on 2017/6/16.
//  Copyright © 2017年 javalong. All rights reserved.
//

#ifndef ConditionTest_hpp
#define ConditionTest_hpp

#include <stdio.h>
#include <pthread.h>

class ConditionTest
{
private:
    static void* thr_fn(void *arg);
    void testCond();
public:
    void testImp();
};

#endif /* ConditionTest_hpp */
