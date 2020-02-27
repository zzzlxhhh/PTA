#include<iostream>
#include<deque>
using namespace std;
int main()
{
    int num;
    cin>>num;
    deque<int> q;
    int temp;
    q.push_back(0);
    for(int i=0;i<num;i++)
    {
        cin>>temp;
        q.push_back(temp);
    }
    int sum=0,dis=0;
    while(q.size()>1)
    {
       if(q[0]>q[1])
        {
            dis=q[0]-q[1];
            sum=sum+dis*4+5;
           // q.pop_top();
        }
        else
        {
            dis=q[1]-q[0];
            sum=sum+dis*6+5;
            //q.pop_top();
        }
        q.pop_front();        
    }
    cout<<sum;
    int pause;
    cin>>pause;

}