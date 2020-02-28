#include<iostream>
#include<string>
#include<set>
#include<vector>
#include<algorithm>
using namespace std;
struct Rank
{
    string ID;
    int C;
    int Math;
    int Eng;
    int Avg;
    int ran;
    friend bool operator==(Rank x,Rank y)
    {
        return (x.ID==y.ID); 
    }
};
bool mypredicte(Rank x,Rank y)
{
    return (x.ID==y.ID);
}
int main()
{
    set<Rank> myset;
    //vector<string> ID_vec;
    //vector<int> idset;
    vector<int> cset;
    vector<int> mathset;
    vector<int> engset;
    vector<int> avgset;
    int n,m;
    cin>>n>>m;
    Rank temp;
    string id;
    int c,math,eng,avg;
    for(int i=0;i<n;i++)
    {
        avg=0;
        cin>>id>>c>>math>>eng;
        temp.ID=id;
        temp.C=c;
        temp.Math=math;
        temp.Eng=eng;
        avg=(math+c+eng)/3;
        temp.Avg=avg;
        cset.push_back(c);
        mathset.push_back(math);
        engset.push_back(eng);
        avgset.push_back(avg);
        myset.insert(temp);
    }
    for(int i=0;i<myset.size();i++)
    {
        sort(cset.begin(),cset.end());
        sort(mathset.begin(),mathset.end());
        sort(engset.begin(),engset.end());
        sort(avgset.begin(),avgset.end());
    }
    vector<Rank>::iterator itr;
    vector<string> query(m);
    for(int i=0;i<m;i++)
    {
        cin>>query[i];
    }
    for(int i=0;i<m;i++)
    {
        id=query[i];
        itr=myset.find(id);
        int max=0;
        if(itr!=myset.end())
        {
            int flag;
            if((*itr).Avg>max)
            {
                max=(*itr).Avg;
                flag=0;
            }
            if((*itr).C>max)
            {
                max=(*itr).C;
                flag=1;
            }
            if((*itr).Math>math)
            {
                max=(*itr).Math;
                flag=2;
            }
            if((*itr).Eng>eng)
            {
                max=(*itr).Eng;
                flag=3;
            }
            vector<int>::iterator it;
            switch (flag)
            {
            case 0:
                it=find(avgset.begin(),avgset.end(),(*itr).Avg);
                cout<<int(1+it-avgset.begin())<<" "<<"A";
            case 1:
                it=find(cset.begin(),cset.end(),(*itr).C);
                cout<<int(1+it-avgset.begin())<<" "<<"C";
            case 2:
                it=find(mathset.begin(),mathset.end(),(*itr).Math);
                cout<<int(1+it-avgset.begin())<<" "<<"M";
            case 3:
                it=find(engset.begin(),engset.end(),(*itr).Eng);
                cout<<int(1+it-avgset.begin())<<" "<<"E";
                /* code */
                break;
            
            default:
                break;
            }
        }
        else cout<<"N/A";
    }
    
}