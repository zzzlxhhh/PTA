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
    int num=0;
    double waittime=0;
    while(q.size()>0)
    {
        if(num<K)
        {
            if(q[0].total<28800)
            {
                waittime+=28800-q[0].total;
                win.push(28800+q[0].tack*60);
                num++;
                q.pop_front();
            }
            else 
            {                
                num++;
                win.push(q[0].total+q[0].tack*60);
                q.pop_front();
            }
        }
        else if(num==K)
        {
            int temp=win.top();
            if(win.top()>q[0].total)
            {
            waittime+=win.top()-q[0].total;
            
            win.push(temp+q[0].tack*60);           
            win.pop();
            }
            else 
            {
            win.push(q[0].total+q[0].tack*60);
            win.pop();
            }
            q.pop_front();
        }
    }
    if(N==0) cout<<"0.0";
    //cout<<waittime;
    printf("%.1f",waittime/60/N);
}