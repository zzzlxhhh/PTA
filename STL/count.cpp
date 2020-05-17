#include <iostream>
#include <vector>
#include <string>
#include<algorithm>
#include <fstream>
#include<time.h>
using namespace std;
vector<char> buf;
vector<pair<string, int>> vec;
bool cmp(const pair<string, int>& a, const pair<string, int>& b)
{
    return a.second > b.second;
}
void fun(string buf)
{
    char ch;
    for (int i = 0; i < buf.size(); i++)
    {
        string s = ""; ch = buf[i];
        while (!((ch >= 'a' && ch <= 'z')||(ch >= 'A' && ch <= 'Z'))) 
        {
            ch = buf[++i];
        }
        if ((ch >= 'a' && ch <= 'z')||(ch >= 'A' && ch <= 'Z'))
        { 
            while ((ch >= 'a' && ch <= 'z')||(ch >= 'A' && ch <= 'Z'))
            {
                if(ch>='a')
                 ch=ch;
                else ch=ch+32;
                s+=ch;
                ch = buf[++i];
            }
            for(int j=0;j<vec.size();j++)
                if(vec[j].first==s) vec[j].second++;
                else if(j==vec.size()-1) 
                vec.push_back(make_pair(s,1));
            i--;
        }
    }
}
int main()
{
    ifstream input("./in.dat");
    string str = "";
    string temp = "";
    clock_t t1=clock();
    if (input.is_open())
    {
        while(getline(input,temp))
        str+=temp;    
    }
    fun(str);

    sort(vec.begin(),vec.end(),cmp);
    for(int i=0;i<3;i++)
    {
        cout<<vec[i].first<<":"<<vec[i].second<<endl;
    }
    cout<< (clock() - t1) * 1.0 / CLOCKS_PER_SEC * 1000<<"ms"<< endl;
    input.close();
    cin>>str;
}
