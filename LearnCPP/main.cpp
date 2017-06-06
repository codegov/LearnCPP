//
//  main.cpp
//  LearnCPlaus
//
//  Created by javalong on 2017/6/1.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include <iostream>
#include "PingTest.cpp"
#include "ClassTest.cpp"
#include "TimeTest.cpp"
#include "SockTest.cpp"
#include "UnionTest.cpp"
#include "IcmpTest.cpp"
#include "SysctlTest.cpp"


int main(int argc, const char * argv[])
{
//    SysctlTest().testImp();
    IcmpTest().testImp();
//    UnionTest().testImp();
//    ClassTest().testImp();
    PingTest().testImp();
//    TimeTest().testImp();
//    SockTest().testImp();
    
    return 0;
}

