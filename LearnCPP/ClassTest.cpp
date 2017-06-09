//
//  ClassTest.cpp
//  LearnCPlaus
//
//  Created by javalong on 2017/6/2.
//  Copyright © 2017年 javalong. All rights reserved.
//
#include <stdio.h>
#include <iostream>

using namespace std;

class Child
{
private:
//    Child(const Child&);
//    Child& operator=(const Child&);
    
public:
    string name()
    {
        return "付**";
    }
    Child()
    {
        cout<< "付**, 起床!\n";
    }
    ~Child()
    {
        cout<< "付**, 睡觉!\n";
    }
};

class Daughter: public Child
{
public:
    static shared_ptr<Daughter>& instance_shared_ptr() { static shared_ptr<Daughter> s_ptr;return s_ptr;}
    string name()
    {
        cout << "Daughter, your name is ";
        return "付一笑";
    }
    Daughter()
    {
        cout<< "付一笑, 起床!\n";
    }
    ~Daughter()
    {
        cout<< "付一笑, 睡觉!\n";
    }
};

class Son: public Child
{
public:
    static shared_ptr<Son>& instance_shared_ptr() { static shared_ptr<Son> s_ptr;return s_ptr;}
    //    string name()
    //    {
    //        cout << "Son, your name is ";
    //        return "付子天";
    //    }
    //    Son()
    //    {
    //        cout<< "付子天, 起床!\n";
    //    }
    //    ~Son()
    //    {
    //        cout<< "付子天, 睡觉!\n";
    //    }
};

class ClassTest
{
public:
    static Child& getChild()
    {
        static Child* mutex = new Child;
        cout << mutex << "\n";
        return *mutex;
    }
    
    void testClass()
    {
        cout << "B="<< &Daughter::instance_shared_ptr() << "  " << &Son::instance_shared_ptr() << "=E\n";
        printf("==%p--%p==\n", &Daughter::instance_shared_ptr(), &Son::instance_shared_ptr());
        //        Daughter d = Daughter();
        //        Son      s = Son();
        //        cout << "B="<< &d << "  " << &s << "=E\n";
        
        cout << "一、Hello, "<< Daughter::instance_shared_ptr()->name() << "\n";
        cout << "二、Hello, "<< Son::instance_shared_ptr()->name() << "\n";
        cout << "三、Hello, "<< Daughter().name() << "\n";
        cout << "四、Hello, "<< Son().name() << "\n";
        
        
        //        Child child;
        //
        getChild();
        getChild();
        getChild();
    }

    void testOther()
    {
        int i1  = 2;
        const int *i2 = &i1;
        //(*i2)++;
        const int &i3 = i1;
        //        i3++;
        //        i3 = 12;
        
        int &i4 = i1;
        i4++;
        i4 = 12;
        
        i1 = 10;
    }
    
    void testRealloc()
    {
        int i;
        int *pn = (int*)malloc(5*sizeof(int));
        printf("malloc %p\n",pn);
        for(i = 0; i < 5; i++) pn[i]=i;
        pn=(int*)realloc(pn,10*sizeof(int));
        printf("realloc%p\n", pn);
        for(i=5;i<10;i++) pn[i]=i;
        for(i=0;i<10;i++) printf("%3d",pn[i]);
        free(pn);
        printf("\n");
    }
    
    void testType()
    {
        int type1 = 1;
        int type2 = 1 << 1;
        int type = type1 | type2;
        if (type & type2)
        {
            printf("操作：int type = type1 | type2;\n");
            printf("type & type2 YES\n");
            type &= ~type2;
            printf("操作：type &= ~type2;\n");
            printf("type & type2 %s\n", type & type2 ? "YES" : "NO");
        }
    }

    void testImp()
    {
        testClass();
        testOther();
        testRealloc();
        testType();
    }
};
