#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include<algorithm>
#include <fstream>
#include<time.h>
using namespace std;
vector<char> chars;
unordered_map<string, int> wordmap;
bool cmp(const pair<string, int>& a, const pair<string, int>& b)
{
    return a.second > b.second;
}
bool isLetter(char ch)
{
	//忽略' 
    if ((ch >= 'a' && ch <= 'z')||(ch >= 'A' && ch <= 'Z'))
        return true;
    else
        return false;
}
void fixVal(string str)
{
    unordered_map<string, int>::iterator itr = wordmap.begin();
    itr = wordmap.find(str);
    if (itr != wordmap.end())
    {
        itr->second++;
    }
    else
    {
        wordmap.insert(make_pair(str, 1));
    }
    return;
}
void token(string chars)
{
    char ch;
    for (int i = 0; i < chars.size(); i++)
    {
        string arr = "";
        ch = chars[i];
        while (!isLetter(ch) && !isNum(ch))
        {
            ch = chars[++i];
        }
        if (isLetter(ch))
        { 
            while (isLetter(ch))
            {
                ch=ch>='a'?ch:ch+32;//忽略大小写
                arr+=ch; //arr的拓展要放在前面
                ch = chars[++i];
            }
            fixVal(arr);
            //注意此处回退一个字符 对应之前++i操作
            i--;
        }
    }
}
int main()
{
    ifstream infile("./myfile.dat");
    string str = "";
    string temp = "";
    clock_t t1=clock();
    if (infile.is_open())
    {
        while(getline(infile,temp))
        str+=temp;    
    }
    token(str);
    vector<pair<string, int>> wordvec(wordmap.begin(),wordmap.end());
    sort(wordvec.begin(),wordvec.end(),cmp);
    for(int i=0;i<3;i++)
    {
        cout<<"top"<<i+1<<":"<<wordvec[i].first<<"#"<<wordvec[i].second<<endl;
    }
    cout<<"time elaspe:"<< (clock() - t1) * 1.0 / CLOCKS_PER_SEC * 1000<<"ms"<< endl;
    infile.close();
    cin>>str;
}
