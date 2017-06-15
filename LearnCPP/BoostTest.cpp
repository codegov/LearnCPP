//
//  BoostTest.cpp
//  LearnCPP
//
//  Created by javalong on 2017/6/14.
//  Copyright © 2017年 javalong. All rights reserved.
//

#include "BoostTest.hpp"
#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

class SignalTest
{
private:
    boost::function<void (string& str)> str_test;
    boost::signals2::signal<void (uint32_t cmdid, const string& str)> str_signal;
    void OnSignalActive()
    {
        printf("1==OnSignalActive==\n");
    }
    void OnSignalActive(uint32_t cmdid, const string& str)
    {
        printf("2==OnSignalActive==%d %s \n", cmdid, str.c_str());
    }
public:
    void testImp()
    {
        str_signal.connect(boost::bind(&SignalTest::OnSignalActive, this));
        str_signal.connect(boost::bind(&SignalTest::OnSignalActive, this, _1, _2));
        str_signal(10, "有两个监听值");
        str_signal.disconnect_all_slots();
        str_signal(11, "没有监听者");
    }
};


class ShareTest
{
private:
    
    typedef boost::function<void ()> AsyncInvokeFunction;
    
    struct Message
    {
        Message(): title(0) {}
        Message(const string& _title, const boost::any& _body1, const boost::any& _body2)
        : title(_title), body1(_body1), body2(_body2) {}
        
        template <class F> Message(const string& _title, const F& _func)
        : title(_title), body1(boost::make_shared<AsyncInvokeFunction>()), body2()
        {
            *boost::any_cast<boost::shared_ptr<AsyncInvokeFunction> >(body1) = _func;
        }
        
        bool operator == (const Message& _rhs) const {return title == _rhs.title;}
        
        string title;
        boost::any body1;
        boost::any body2;
    };
    
    static void __AsyncInvokeHandler(const uint32_t& _id, Message& _message)
    {
        (*boost::any_cast<boost::shared_ptr<AsyncInvokeFunction> >(_message.body1))();
    }
    
    static void doAction1()
    {
        printf("====doAction1====\n");
    }
    
    static void doAction2()
    {
        printf("====doAction2====\n");
    }
    
    void doAction3()
    {
        printf("====doAction3====\n");
    }
    
#define SET_BIT(IS_TRUE, RECORDS, VALID_BITS) \
do \
{ \
RECORDS <<= 1; \
RECORDS &= VALID_BITS; \
if (IS_TRUE) \
RECORDS |= 1; \
else \
RECORDS &= ~1; \
} \
while(false)
    
    //calc how many 1 in records
#define CAL_BIT_COUNT(RECORDS, COUNT) \
do \
{ \
uint32_t eva = RECORDS; \
while(eva) \
{ \
eva = eva & (eva-1); \
COUNT++; \
} \
} \
while(false)
    
    //is the last n bits all zero
#define IS_LAST_N_BIT_ZERO(RECORDS, N, RET) \
do \
{ \
uint32_t N_ = N; \
if (N_>32) {N_=0; }\
uint32_t mask = (0xFFFFFFFF >> (32-N_)); \
RET = (0==((RECORDS & mask) ^ 0x00000000));\
} \
while(false)
    
    //extact the N bits from RECORDS, which is started by START_POS, and return the result by RETURN_RECORD
#define EXTRACT_N_BITS(RECORDS, START_POS, N, RETURN_RECORD) \
do \
{ \
uint32_t N_ = (N); \
uint32_t START_POS_ = (START_POS);\
if (START_POS_>32 || START_POS_<1) {START_POS_=1;} \
if (START_POS_+N_>32) {N_=32-START_POS_;}\
RETURN_RECORD = ((RECORDS << (START_POS_-1)) >> (32-N_)); \
}\
while(false)
    
    void testByte()
    {
        static const uint32_t kValidBitsFilter = 0xFFFFFFFF;  //32 bits stands for recent 32 tasks status
        static const uint32_t kMostRecentTaskStartN[2]			={24, 8};
        static const uint32_t kSecondRecentTaskStartN[2] 		={16, 8};
        //static const uint32_t MOST_OLD_TASK_START_N[2]				={1, 16};
        
        static const uint32_t kCheckifBelowCount = 3;
        static const uint32_t kCheckifAboveCount = 5;
        uint32_t records = 0xFFFFFFFD;
        
        uint32_t succ_count = 0;
        uint32_t most_recent_shorttasks_status = 0;
        
        EXTRACT_N_BITS(records, kMostRecentTaskStartN[0], kMostRecentTaskStartN[1], most_recent_shorttasks_status);
        printf("======%u\n", most_recent_shorttasks_status);
        CAL_BIT_COUNT(most_recent_shorttasks_status, succ_count);
        printf("======%u\n", succ_count);
        unsigned int valid_record_taskcount = 0;
        CAL_BIT_COUNT(kValidBitsFilter, valid_record_taskcount);
        printf("======%u\n", valid_record_taskcount);
    }
    
public:
    void testImp()
    {
        testByte();
        
        string str = "Hello World!";
        Message message1(str, doAction1);
        __AsyncInvokeHandler(11, message1);
        
        Message message2(str, doAction2);
        __AsyncInvokeHandler(12, message2);
        
        Message message3(str, boost::bind(&ShareTest::doAction3, this));
        __AsyncInvokeHandler(13, message3);
    }
};

void BoostTest::testImp()
{
    SignalTest().testImp();
    ShareTest().testImp();
}

