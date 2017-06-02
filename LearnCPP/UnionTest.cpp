//
//  UnionTest.cpp
//  LearnCPP
//
//  Created by javalong on 2017/6/2.
//  Copyright © 2017年 javalong. All rights reserved.
//


/**
 
 union  ( 共用体 )：构造数据类型,也叫联合体
 用途：使几个不同类型的变量共占一段内存(相互覆盖)
 
 struct ( 结构体 )：是一种构造类型
 用途：把不同的数据组合成一个整体——自定义数据类型
 
 主要区别：
 1. struct和union都是由多个不同的数据类型成员组成, 但在任何同一时刻, union中只存放了一个被选中的成员; 而struct的所有成员都存在。在struct中，各成员都占有自己的内存空间，它们是同时存在的,一个struct变量的总长度等于所有成员长度之和，遵从字节对其原则; 在Union中，所有成员不能同时占用它的内存空间，它们不能同时存在, Union变量的长度等于最长的成员的长度。
 
 2. 对于union的不同成员赋值, 将会对其它成员重写, 原来成员的值就不存在了,所以，共同体变量中起作用的成员是最后一次存放的成员; 而对于struct的不同成员赋值是互不影响的。
 
 */

/**
 %a,%A 读入一个浮点值(仅C99有效)
 %c 读入一个字符
 %d 读入十进制整数
 %i 读入十进制，八进制，十六进制整数
 %o 读入八进制整数
 %x,%X 读入十六进制整数
 %s 读入一个字符串，遇空格、制表符或换行符结束。
 %f,%F,%e,%E,%g,%G 用来输入实数，可以用小数形式或指数形式输入。
 %p 读入一个指针
 %u 读入一个无符号十进制整数
 %n 至此已读入值的等价字符数
 %[] 扫描字符集合
 %% 读%符号
 */

#include <stdio.h>
#include <iostream>

using namespace std;

class UnionTest
{
public:
    void testImp()
    {
        union
        {
            int i;            // 4Byte
            struct
            {
                char first;   // bit7..bit0
                char second;  // bit15..bit8
                char third;   // bit23..bit16
                char fourth;  // bit24..bit31
            }half;
        } number;
        
        number.i = 0x87654321;
        printf("union half1=%x %x %x %x\n", number.half.first, number.half.second, number.half.third, number.half.fourth);
        printf("union half2=%d %d %d %d\n", number.half.first, number.half.second, number.half.third, number.half.fourth);
        cout <<"union half3="<<number.half.first<< " "<<number.half.second<<" "<<number.half.third<<" "<<number.half.fourth<<"\n";
        
        number.half.first  = 1;
        number.half.second = 2;
        number.half.third  = 4;
        number.half.fourth = 8;
        
        printf("union i=%x\n", number.i);
    }
};
