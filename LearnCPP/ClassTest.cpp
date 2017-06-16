//
//  ClassTest.cpp
//  LearnCPlaus
//
//  Created by javalong on 2017/6/2.
//  Copyright © 2017年 javalong. All rights reserved.
//
#include <stdio.h>
#include <iostream>
#include <list>
#include <map>

using namespace std;

typedef function<void (string &id, string &body)> TestHandler;

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
//    Child()
//    {
//        cout<< "付**, 起床!\n";
//    }
//    ~Child()
//    {
//        cout<< "付**, 睡觉!\n";
//    }
};

class Daughter: public Child
{
    string _name;
public:
    static shared_ptr<Daughter>& instance_shared_ptr() { static shared_ptr<Daughter> s_ptr;return s_ptr;}
    string name()
    {
        cout << "Daughter, your name is ";
        return _name;
    }
    Daughter()
    {
        cout<< _name << ", 起床!\n";
    }
    Daughter(string name = "付一笑")
    {
        _name = name;
        cout<< _name << ", 起床!\n";
    }
    ~Daughter()
    {
        cout<< _name << ", 睡觉!\n";
    }
};

class Son: public Child
{
public:
    static shared_ptr<Son>& instance_shared_ptr() { static shared_ptr<Son> s_ptr;return s_ptr;}
        string name()
        {
            cout << "Son, your name is ";
            return "付子天";
        }
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
    list<string> tempList_;
    
    Daughter daugther_;
    string   name_;
    
public:
    
    ClassTest():daugther_("付优"), name_(Son::instance_shared_ptr()->name())
    {
        printf("%s\n", name_.c_str());
    }
    
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
//        cout << "三、Hello, "<< Daughter().name() << "\n";
        cout << "四、Hello, "<< Son().name() << "\n";
        
        
        Child child; // 执行构造函数
        
        getChild();//0x100201f90
        getChild();//0x100201f90
        getChild();//0x100201f90 同一个对象
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
    
    static bool __compare_list(const string& first, const string& second)
    {
        return first < second;
    }

    void testList()
    {
        list<string> tlist = tempList_;
        tempList_.clear();
        tlist.sort(__compare_list);
        
        for (std::list<string>::iterator it=tlist.begin(); it != tlist.end(); ++it)
        {
            printf("======%s\n", it->c_str());
        }
    }

    #define lc_classtest_map classtest_map()
    static std::map<int, string>& classtest_map() {
        static std::map<int, string>* mq_map = new std::map<int, string>;
        return *mq_map;
    }
    
    void testMap()
    {
        int id = 1;
        if (lc_classtest_map.end() == lc_classtest_map.find(id))
        {
            string& content = lc_classtest_map[id];
            content = "12312";
        }
        string &cc = lc_classtest_map[id];
        printf("map===%s\n", cc.c_str());
    }
    
    void testImp()
    {
        
//        testClass();
//        testOther();
//        testRealloc();
//        testType();
//        
//        tempList_.push_back("17");
//        tempList_.push_back("12");
//        tempList_.push_back("11");
//        tempList_.push_back("14");
//        tempList_.push_back("13");
//        testList();
//        
//        testMap();
//        testMap();
    }
};
