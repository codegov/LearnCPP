//
//  runable.h
//  LearnCPP
//
//  Created by javalong on 2017/6/19.
//  Copyright © 2017年 javalong. All rights reserved.
//

#ifndef runable_h
#define runable_h

struct Runnable
{
    virtual ~Runnable() {}
    virtual void run() = 0;
};

namespace detail
{
    
    template <class T>
    class RunnableFunctor : public Runnable
    {
    public:
        RunnableFunctor(const T& f) : func_(f)
        {
            printf("===RunnableFunctor===\n");
        }
        virtual void run()
        {
            func_();
            printf("===RunnableFunctor===run\n");
        }
    private:
        T func_;
    };
    
    template <class T>
    class RunnableFunctor<T*> : public Runnable
    {
    public:
        RunnableFunctor(T* f) : func_(f)
        {
            printf("===RunnableFunctor<T*>===\n");
        }
        virtual void run()
        {
            (*func_)();
            printf("===RunnableFunctor<T*>===run\n");
        }
        
    private:
        RunnableFunctor(const RunnableFunctor&);
        RunnableFunctor& operator=(const RunnableFunctor&);
        
    private:
        T* func_;
    };
    
    template <>
    class RunnableFunctor<Runnable> : public Runnable
    {
        RunnableFunctor();
    };
    
    template <>
    class RunnableFunctor<Runnable*> : public Runnable
    {
    public:
        RunnableFunctor(Runnable* f) : func_(f)
        {
            printf("===RunnableFunctor<Runnable*>===\n");
        }
        virtual void run()
        {
            static_cast<Runnable*>(func_)->run();
            printf("===RunnableFunctor<Runnable*>===run\n");
        }
        
    private:
        RunnableFunctor(const RunnableFunctor&);
        RunnableFunctor& operator=(const RunnableFunctor&);
        
    private:
        Runnable* func_;
    };
    
    // base template for no argument functor
    template <class T>
    struct TransformImplement
    {
        static Runnable* transform(const T& t)
        {
            return new RunnableFunctor<T>(t);
        }
    };
    
    template <class T>
    inline Runnable* transform(const T& t)
    {
        return TransformImplement<T>::transform(t);
    }
    
}  // namespace detail


#endif /* runable_h */
