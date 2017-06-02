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
private:
    void testUnoin()
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
        
        printf("union i=%x  %ld  %ld\n", number.i, sizeof(number), sizeof(number.half));
    }
    
    void testStructSize()
    {
        struct
        {
            char c1;   // 1字节
            char c2;   // 1字节
            u_short i; // 2字节
            
            union
            {
                u_short u1;
            }UU1; // 2字节
            
            union
            {
                long u2;
            }UU2; // 8字节
        }SS; // 16字节
        
        /**
         １:数据成员对齐规则：结构(struct)(或联合(union))的数据成员，第一个数据成员放在offset为0的地方，以后每个数据成员存储的起始位置要从该成员大小或者成员的子成员大小（只要该成员有子成员，比如说是数组，结构体等）的整数倍开始(比如int在32位机为４字节,则要从４的整数倍地址开始存储。
         
         ２:结构体作为成员:如果一个结构里有某些结构体成员,则结构体成员要从其内部最大元素大小的整数倍地址开始存储.(struct a里存有struct b,b里有char,int,double等元素,那b应该从8的整数倍开始存储.)
         
         ３:收尾工作:结构体的总大小,也就是sizeof的结果,.必须是其内部最大成员的整数倍.不足的要补齐.
         */
        
        typedef struct bb
        {
            int id;             //[0]....[3]
            double weight;      //[8]........[15]　原则１ 因为double的大小是8字节却不是第一个数据，存储的起始位置要从8的整数倍开始，则从8字节开始存储
            float height;       //[16]....[19]
        }BB;                    //内部成员大小最大的为double，大小为8,补齐字节[20]...[23]  原则３
        
        typedef struct aa
        {
            char name[2];     //[0].[1]
            int  id;          //[4]....[7]　　　　原则１
            
            double score;     //[8]....[15]
            short grade;      //[16].[17]
            BB b;             //[24]......[47]　 原则２ 结构体成员要从其内部最大元素大小的整数倍地址开始存储
        }AA;                  //内部成员大小最大的为BB，大小为24,补齐字节[48]  原则３
        
        printf("内存字节对齐=%ld %ld\n", sizeof(BB), sizeof(AA));
        
        printf("struct size:%ld %ld %ld\n", sizeof(SS), sizeof(SS.UU1), sizeof(SS.UU2));
    }
    
public:
    void testImp()
    {
        testUnoin();
        testStructSize();
    }
};
