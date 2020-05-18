#define MaxVerNum 100
#define INFINITY 65535
#include<iostream>
using namespace std;
int r[MaxVerNum][MaxVerNum];
int path[MaxVerNum][MaxVerNum];
void print_midpath(int i,int j)
{
    int tmp=path[i][j];
    if(tmp!=-1)
    {
        print_midpath(i,tmp);
        cout<<tmp<<"->";
        print_midpath(tmp,j);
    }   
}
int main()
{
    int v_num,e_num;
    cin>>v_num>>e_num; 
    //所有的边不可达初始化为infinity
    for(int n=0;n<v_num;n++)
        for(int p=0;p<v_num;p++)
        r[n][p]=INFINITY;
    int i,j,w;//输入边
    for(int n=0;n<e_num;n++)
    {
        cin>>i>>j>>w;
        r[i][j]=w;
        r[j][i]=w;
        path[i][j]=-1;
        path[j][i]=-1;
    }
    //为进行floyd算法做初始化
    //对角元素初始化为0
    for(i=0;i<v_num;i++)
        r[i][i]=0;
    
    //floyd算法：实际为动态规划
    //多源最短路算法
    for(int k=0;k<v_num;k++)
    for(i=0;i<v_num;i++)
    for(j=0;j<v_num;j++)
    {
        if(r[i][k]+r[k][j]<r[i][j])
        {
            r[i][j]=r[i][k]+r[k][j];
            if(i==j&&r[i][j]<0)//负数环
            return 0;
            path[i][j]=k;
        }
    }
   // int b=65535;
    //cout<<b;
    cout<<0<<"->";
    print_midpath(0,3);
    cout<<3;
    cin>>i;
}