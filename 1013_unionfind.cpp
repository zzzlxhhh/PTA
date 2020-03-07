#include<iostream>
#include<vector>
using namespace std;
vector<int> e[2];
int N,M,K;
int *g;
int *s;
int root(int p)
{

    //递归实现 路径压缩
    if(g[p]!=p) g[p]=root(p);
    return g[p];//chase parent pointers

   /* int par;
    par=g[p];
    if(g[p]!=p)
    {
        while(g[par]!=par)
        par=g[par];
        g[p]=par;
    }
    return par;
*/
}
void link(int p,int q)
{
    int i=root(p);
    int j=root(q);
    if(i==j) return ; //important
    g[i]=j;
   /* if(s[i]<s[j])
    {
        g[i]=j;
        s[j]=s[i]+s[j];
    }//make it weighted
    else
    {g[j]=i;s[i]+=s[j];}*/ 
}
void remake()
{
    for(int i=0;i<=N;i++)
     {
         s[i]=1;
         g[i]=i;
     }     
}
int main()
{
    cin>>N>>M>>K;
    int t1,t2;
    g=(int *)malloc(sizeof(int)*(N+1));
    s=(int *)malloc(sizeof(int)*(N+1));
    for(int i=0;i<M;i++)
    {
        cin>>t1>>t2;
        e[0].push_back(t1);
        e[1].push_back(t2);
    }
    for(int i=0;i<K;i++)
    {
        cin>>t1;
        remake();
        int count=0;
        for(int j=0;j<M;j++)
        {
            if(t1!=e[0][j]&&t1!=e[1][j])
            {
                link(e[0][j],e[1][j]);
            }
        }
        for(int n=1;n<=N;n++)
        if(g[n]==n) count++;

        cout<<(count-2)<<endl;
    }
    int pause;
    cin>>pause;
}