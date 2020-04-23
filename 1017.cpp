#include<iostream>
#include<deque>
#include<queue>
#include<algorithm>
using namespace std;
struct  date
{
    int hh;
    int mm;
    int ss;
    int tack;
    int total;
    int wait=0;
    /* data */
};
deque<date> q;
bool cmp(date a,date b)
{
    return a.total<b.total;
}
int main()
{
    int N,K;
    cin>>N>>K;
    for(int i=0;i<N;i++)
    {
        date temp;
        scanf("%d:%d:%d",&temp.hh,&temp.mm,&temp.ss);
        temp.total=(temp.hh*60+temp.mm)*60+temp.ss;
        cin>>temp.tack;
        if(temp.total<61201)
        q.push_back(temp);
    }
    N=q.size();
    sort(q.begin(),q.end(),cmp);
    priority_queue<int,vector<int>,greater<int>> win;
    double waittime=0;
    for(int i=0;i<K;i++) win.push(28800);
    while(q.size()>0)
    {
        if(q[0].total<win.top())
            {
                waittime+=win.top()-q[0].total;
                
                win.push(win.top()+q[0].tack*60);
                win.pop();
                q.pop_front();
            }
        else 
        {
            //waittime+=win.top()-q[0].total;
            win.pop();
            win.push(q[0].total+q[0].tack*60);
            //zhuyi
            q.pop_front();
        }
    }
    if(K==0) cout<<"0.0";
    //cout<<waittime;
    printf("%.1f",waittime/60/N);
}