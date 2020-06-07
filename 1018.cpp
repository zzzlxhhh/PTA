#include<iostream>
#include<vector>
#define infinity 6553555
using namespace std;
int max_c,sta_num,sP,road_num;
int t[501][501];
int dist[501];
vector<int> path[501];
int main()
{
    cin>>max_c>>sta_num>>sP>>road_num;
    int tmpi,tmpj;
    int *cap=(int*)malloc((sta_num+1)*sizeof(int));
    for(int i=1;i<sta_num+1;i++)
    cin>>cap[i];
    for(int i=0;i<sta_num+1;i++)
        for(int j=0;j<sta_num+1;j++)
       {
        t[i][j]=-1;
        t[j][i]=-1;
       } 
    for(int i=0;i<road_num;i++)
    {
        cin>>tmpi>>tmpj;
        cin>>t[tmpi][tmpj];
        t[tmpj][tmpi]=t[tmpi][tmpj];
    }
    //先进行dji算法
    fill(dist[0],dist[501],infinity);
    

}