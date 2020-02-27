#include<iostream>
#include<vector>
using namespace std;
int main()
{
    int num,temp;
    vector<int> v;
   // vector<int> q;
    cin>>num;
    for(int i=0;i<num;i++)
    {
        cin>>temp;
        v.push_back(temp);
    }
    int right=0,left=0;
    int lefttemp=0;
    int thisSum=0,max=-1;//注意最终的输出
    for(int i=0;i<num;i++)
    {
        thisSum+=v[i];
        if(thisSum>max)//保证index故 不是>=
        {
            max=thisSum;
            right=i;
            left=lefttemp;
            //q.push_back(v[i]);
        }
        else if(thisSum<0)//注意此处 要保证index
        {
            //q.clear();
            thisSum=0;
            lefttemp=i+1<num? i+1:i;
        }
    }
    //int i=q.size();
    if(max>=0)
    cout<<max<<" "<<v[left]<<" "<<v[right];
    else
    {
        cout<<0<<" "<<v[0]<<" "<<v[num-1];
    }
    
    int pause;
    cin>>pause;
}