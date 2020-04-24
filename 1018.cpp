#include<iostream>

using namespace std;
int max_cap,sta_num,sP,road_num;
int t[501][501];
int main()
{
    cin>>max_cap>>sta_num>>sP>>road_num;
    int tmpi,tmpj;
    int *cap=(int*)malloc(sta_num*sizeof(int));
    for(int i=0;i<sta_num;i++)
    cin>>cap[i];
    for(int i=0;i<road_num;i++)
    {
        cin>>tmpi>>tmpj;
        cin>>t[tmpi][tmpj];
        t[tmpj][tmpi]=t[tmpi][tmpj];
    }
    

}