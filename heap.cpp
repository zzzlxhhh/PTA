#include<iostream>
using namespace std;
struct heapnode{
    int *elems;
    int size;
    int cap;//堆的最大容量
};
/*
对于删除是末节点 下滤
对于插入 上滤
构建和删除类似 有下滤 对于下滤的节点即使有swap其左右树都是堆 所以可行
只需挑出左右队中更大的一个swap即可
*/
heapnode* createheap(int maxsize)
{
    heapnode* h=(heapnode *)malloc(sizeof(heapnode));
    h->size=0;
    h->cap=maxsize;
    h->elems=(int *)malloc((maxsize+1)*sizeof(int));
    h->elems[0]=99999;//哨兵 ，方便插入的时候做判断，否则要另外注意根节点的问题
    return h;
}
bool insert(heapnode *h,int x)
{
    int i;
    //堆已满
    if(h->size==h->cap) return 0;
    i=++h->size;
    for(;h->elems[i/2]<x;i=i/2)
        h->elems[i]=h->elems[i/2];
    h->elems[i]=x;
    return 1;
}
int DeleteMax(heapnode *h)//删除根，并返回值
{
    //从最后一个节点替补后，向下调整
    int par,ch;
    int max,tmp;
    max=h->elems[1];
    tmp=h->elems[h->size--];
    for(par=1;par*2<=h->size;par=ch)
    {//par*2为左儿子，若超过size，则没有儿子节点
        ch=par*2;
        //有右儿子
        if(ch!=h->size&&h->elems[ch]<h->elems[ch+1])
            ch++;//右节点更大 指向右节点
        if(tmp>=h->elems[ch]) break;//找到合适的点
        else
            h->elems[par]=h->elems[ch];
        
    }
    h->elems[par]=tmp;
    return max;
}
/*给定序列建立heap*/
//p就相当于是delete中的末节点
void build(heapnode *h,int p)
{
    int tmp=h->elems[p];
    int ch,par;
    for(par=p;par*2<=h->size;par=ch)
    {
        ch=par*2;
        if(ch!=h->size&&h->elems[ch]<h->elems[ch+1])
        ch++;
        if(tmp>=h->elems[ch]) break;
        else h->elems[par]=h->elems[ch];
    }
    h->elems[par]=tmp;
}
void BuildHeap(heapnode *h)
{
    for(int i=h->size/2;i>=1;i--)
    {
        build(h,i);
    }
}
void printheap(heapnode *h)
{
    for(int i=1;i<=h->size;i++)
        cout<<h->elems[i]<<"->";
}
int main()
{
    int tmp;
    heapnode *h1=createheap(100);
    for(int i=0;i<8;i++)
    {
        cin>>tmp;
        insert(h1,tmp);
    }
    //cout<<"最大："<<DeleteMax(h1)<<endl;
    printheap(h1);
    cout<<endl;
    heapnode *h2=createheap(100);
    for(int i=1;i<=8;i++)
    {
        cin>>h2->elems[i];
        h2->size++;
    }
    BuildHeap(h2);
    printheap(h2);
    tmp=0;
}
