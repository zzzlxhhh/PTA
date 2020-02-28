#include<iostream>
#include<vector>
#include<queue>
#include<deque>
#include<iomanip>
using namespace std;

int main()
{
    vector<double> r[3];
    double w,t,l;
    deque<int> q2;
    queue<double> q1;
    for(int i=0;i<3;i++)
    {
        cin>>w>>t>>l;
        r[i].push_back(w);
        r[i].push_back(t);
        r[i].push_back(l);
    }
    for(int i=0;i<3;i++)
    {
        double max=0;
        q2.push_back(max);
        int j;
        for(j=0;j<3;j++)
        {
            if(max<r[i][j])
            {
                max=r[i][j];
                q2.pop_back();
                q2.push_back(j);    
            }
        }
        q1.push(max);
        //q2.push(--j);
    }
    double total=1;
    while(!q1.empty())
    {
        total*=q1.front();
        q1.pop();
    }
    total=total*0.65*2-2;
    while(!q2.empty())
    {
        switch (q2.front())
        {
        case 0:
            cout<<"W";
            break;
        case 1:
            cout<<"T";
            break;
        case 2:
            cout<<"L";
            break;
        default:
            break;
        }
        cout<<" ";
        q2.pop_front();
    }
    cout<<fixed<<setprecision(2)<<total;
    int pause;
    cin>>pause;
}