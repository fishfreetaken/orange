
#include "anysort.h"




template<typename T>
anysort<T>::anysort(size_t n):count_(n),
buf_(nullptr)
{
   buf_= (T*)malloc(sizeof(T)*n);
    memset(buf_,0,sizeof(T)*n);

    pri = new priqueue<T>(n);
}

template<typename T>
anysort<T>::~anysort()
{
    free(buf_);
    count_=0;

    delete pri;
}

template<typename T>
void anysort<T>::quicksub(T*buf,int begin,int end)
{
    if((begin>=end)||(begin<0)||(end<0))
    {
         return;
    }

    T tmp = buf[end];
    int div=begin-1;
    for(int i=begin;i<end;i++)
    {
        if(buf[i]<=tmp)
        {
            div++;
            std::swap(buf[div],buf[i]);
        }
    }
    div++;
    std::swap(buf[div],buf[end]);

    quicksub(buf,begin,div-1);
    quicksub(buf,div+1,end);
}

template<typename T>
void anysort<T>::insertsort(T *buf,size_t n)
{
    if((n==0)||(buf==nullptr))
    {
        return ;
    }
    
    for(int i=1;i<n;i++)
    {
        int j=i-1;
        while((buf[j]>buf[j+1])&&(j>=0))
        {
            std::swap(buf[j],buf[j+1]);
            j--;
        }
    }
}

template<typename T>
void anysort<T>::quicksort(T *buf,size_t n)
{
    if((n==0)||(buf==nullptr))
    {
        return ;
    }
    quicksub(buf,0,n-1);
}

template<typename T>
int anysort<T>::printTopk(size_t k)
{
    if(k>count_)
    {
        return -1;
    }
    printf("This is top k result %ld\n",k);
    if(pri == nullptr)
    {
        for(int i=0;i<k;i++)
        {
            printf("%x ",buf_[i]);
        }
        printf("\n");
    }else{
        pri->sort(k);
    }
    return k;
}

template<typename T>
int anysort<T>::resultTopk(size_t k,std::vector<T>&top)
{
    if(k>count_)
    {
        return -1;
    }
    printf("This is top k result %ld\n",k);

    if(pri!=nullptr)
    {
        pri->sortpt(buf_,k);
    }

    T buf[k];
    int i=0,j=0;
    for(size_t t=0;t<k;t++)
    {
        if(top[j]<buf_[i])
        {
            buf[t]=buf_[i];
            i++;
        }else{
            buf[t]=top[j];
            j++;
        }
        //printf("%x %x\n",buf_[count_-t-1],top[t]);
    }
    for(i=0;i<10;i++)
    {
        top[i]=buf[i];
    }
}

template<typename T>
void anysort<T>::topk(T *buf,size_t n)
{
    if((n==0)||(buf==nullptr))
    {
        return ;
    }
 
    quicksort(buf,n);
    
    if(buf[n-1]<=buf_[0])
    {
        return ;
    }
    if(buf[0]>=buf_[count_-1])
    {
        memcpy(buf_,buf,sizeof(T)*std::min(n,count_));
        return;
    }
    T tmp[count_];
    size_t a=count_-1;
    n--;
    for(int i=count_-1;i>=0;i--)
    {
        if(buf[n]>buf_[a])
        {
            tmp[i]=buf[n];
            n--;
        }else if(buf[n]<buf_[a]){
            tmp[i]=buf_[a];
            a--;
        }else{
            tmp[i]=buf_[a];
            a--;
            n--;
        }
    }
    memcpy(buf_,tmp,sizeof(T)*count_);
}

template<typename T>
 void anysort<T>::benchmark(T *tbuf,size_t n)
{
    T buf[n];
    //T* buf= (T*)malloc(sizeof(T)*n);
    memcpy(buf,tbuf,sizeof(T)*n);

    timebench t;
    t.begin();

    insertsort(buf,n);
    t.end();
    
    quicksort(tbuf,n);
    t.end();

    int cc=0;
    for(int i=0;i<n;i++)
    {
        if(buf[i]==tbuf[i])
        {
            printf("%x %x\n",buf[i],tbuf[i]);
            cc++;
        }
    }
    printf("total %ld: same:%d unsame:%ld\n",n,cc,n-cc);
    //free(buf);
}

template<typename T>
void anysort<T>::stacktopk(T *tbuf,size_t n)
{
    if((n==0)||(tbuf==nullptr))
    {
        return ;
    }
    for(int i=0;i<n;i++)
    {
        pri->swap(*(tbuf+i));
    }
}

void priquesorttest()
{
    unsigned int arr[]={56,15,8,231,98,2,98,15,45,98,458,12,47};
    priqueue<unsigned int> t(sizeof(arr)/4);

    for(auto i : arr)
    {
        t.swap(i);
    }
    t.printpt();
/*
    t.sort(arr,sizeof(arr)/4);
    for(auto i : arr)
    {
        printf("%u ",i);
    }
    printf("\n");*/
}

template class anysort<unsigned int>;

