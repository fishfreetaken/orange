
#ifndef ANY_SORT_
#define ANY_SORT_
//#include "debug.h"
#include "randomkey.h"

void priquesorttest();

template<typename T>
class priqueue{
public:
    priqueue(size_t n):
    n_(0),
    size_(n)
    {
        pt =new T[n];
    }

    ~priqueue()
    {
        delete [] pt;
    }
    
    T top()
    {
        return *pt;
    }

    size_t size()
    {
        return n_;
    }

    size_t build(T *p,size_t n)
    {
        if(n>size_)
        {
            printf("error put size is too large! exceed %lu -- %lu\n",n_,n);
            return 0;
        }
        for(int i=0;i<n;i++)
        {
            pt[i]=p[i];
        }
        n_=n;
        for(int i = n/2;i>=0;i--)
        {
            rebuild(pt,i);
        }
        
        return n;
    }

    size_t swap(T k) //swap the top,if not full and insert into it
    {
        if(n_>=size_)
        {
            if(k>pt[0])
            {
                pt[0]=k;
                rebuild(pt,0);
            }
            return n_;
        }
        pt[n_]=k;
        size_t i=n_;

        while ((i>0)&&(pt[parent(i)]>pt[i]))
        {
            std::swap(pt[parent(i)],pt[i]);
            i=parent(i);
        }
        n_++;
        return n_;
    }

    void sort(T *p,size_t n)
    {
        build(p,n);
        for(int i = n_-1;i>0;i--)
        {
            std::swap(pt[0],pt[i]);
            n_--;
            rebuild(pt,0);
        }
        for(int i=0;i<n;i++)
        {
            p[i]=pt[i];
        }
    }


    void printpt()
    {
        for(int i=0;i<size_;i++)
        {
            printf("%u ",pt[i]);
        }

        printf("\n");
    }

    void sort(size_t n)
    {
        for(int i = n_-1;i>0;i--)
        {
            std::swap(pt[0],pt[i]);
            n_--;
            rebuild(pt,0);
        }
        for(int i=0;i<n;i++)
        {
            printf("%x " ,pt[i]);
        }
        printf("\n");
    }
    void sortpt(T *vs, size_t n)
    {
        sort(n);
        for(int i=0;i<n;i++)
        {
            vs[i]=pt[i];
        }
    }
private:

    
    size_t left(size_t i)
    {
        if(i==0)
        {
            return 1;
        }
        return i*2;
    }
    size_t right(size_t i)
    {
        if(i==0)
        {
            return 2;
        }
        return i*2+1;
    }
    size_t parent(size_t i)
    {
        return i/2;
    }
    

    void rebuild(T *pt,size_t i)
    {
        size_t l = left(i);
        size_t r= right(i);

        size_t smin=i;
        if((l<n_)&&(pt[smin]>pt[l]))
        {
            smin=l;
        }
        if((r<n_)&&(pt[smin]>pt[r]))
        {
            smin=r;
        }
        if(smin!=i)
        {
            std::swap(pt[i],pt[smin]);
            rebuild(pt,smin);
        }
    }
private:
    size_t n_;
    const size_t size_;
    T *pt;
};


template<typename T>
class anysort{
public:
    anysort()=delete;

    anysort(size_t n);
    ~anysort();
    
    void topk(T *buf,size_t n);

    int printTopk(size_t k);

    int resultTopk(size_t k,std::vector<T>&top);

    void benchmark(T *tbuf,size_t n);

    void stacktopk(T *buf,size_t n);

    void lookstack()
    {
        pri->printpt();
    }
private:

    void insertsort(T *buf,size_t n);

    void quicksort(T *buf,size_t n);
    void quicksub(T*buf,int begin,int end);

    void stacksort(T *buf,size_t n);

private:
    size_t count_;
    T *buf_;

    priqueue<T> *pri;

};



#endif