//
//  TestRunable.cpp
//  LearnCPP
//
//  Created by javalong on 2017/6/19.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include "TestRunable.hpp"
#include "Runable.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace std;

void TestRunable::testRun(int i)
{
    printf("======testRun======%d\n", i);
}

static void testRun2()
{
    printf("======testRun2======\n");
}

static void testRun3(string &str)
{
    printf("======testRun3======%s\n", str.c_str());
}

void TestRunable::testImp()
{
    Runnable* target_ = detail::transform(boost::bind(&TestRunable::testRun, this, 1212));
    target_->run();
    
    printf("\n\n");
    
    Runnable* target2 = detail::transform(&testRun2);
    target2->run();
    
    printf("\n\n");
    
    string str = "1231sdsddss";
    Runnable* target3 = detail::transform(boost::bind(&testRun3, str));
    target3->run();
    
    printf("\n\n");
    
    boost::function<string ()> block1;
    string findS = "aa";
    int i = 122;
    block1 = [&findS, i]() {
        printf("1findS=%s %d\n", findS.c_str(), i);
        return "sssds";
    };
    Runnable* target4 = detail::transform(block1);
    target4->run();
    
    
}
