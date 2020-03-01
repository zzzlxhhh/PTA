#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
using namespace std;
struct Rank
{
    string ID;
    double C;
    double Math;
    double Eng;
    double Avg;
    int ran;
    friend bool operator==(Rank x, Rank y)
    {
        return (x.ID == y.ID);
    }
};
vector<double> cset;
vector<double> mathset;
vector<double> engset;
vector<double> avgset;
vector<Rank> myset;
bool mypredicte(Rank x, Rank y)
{
    return (x.ID == y.ID);
}
vector<Rank>::iterator find_id(string str)
{
    vector<Rank>::iterator itr;
    for (itr = myset.begin(); itr < myset.end(); itr++)
        if ((*itr).ID == str)
            return itr;
    return itr;
}
char find_top(vector<Rank>::iterator itr)
{
    vector<double>::iterator it;

    char temp;
    int top = 2001;

    it = find(avgset.begin(), avgset.end(), (*itr).Avg);
    top = 1 + it - avgset.begin();
    temp = 'A';
    (*itr).ran = top;

    it = find(cset.begin(), cset.end(), (*itr).C);
    if ((1 + it - cset.begin()) < top)
    {
        temp = 'C';
        (*itr).ran = (1 + it - cset.begin());
        top = (*itr).ran;
    }

    it = find(mathset.begin(), mathset.end(), (*itr).Math);
    if ((1 + it - mathset.begin()) < top)
    {
        temp = 'M';
        (*itr).ran = (1 + it - mathset.begin());
        top = (*itr).ran;
    }

    it = find(engset.begin(), engset.end(), (*itr).Eng);
    if ((1 + it - engset.begin()) < top)
    {
        temp = 'E';
        (*itr).ran = (1 + it - engset.begin());
        top = (*itr).ran;
    }

    return temp;
}
int main()
{
    int n, m;
    cin >> n >> m;
    Rank temp;
    string id;
    int c, math, eng, avg;
    for (int i = 0; i < n; i++)
    {
        avg = 0;
        cin >> id >> c >> math >> eng;
        temp.ID = id;
        temp.C = c;
        temp.Math = math;
        temp.Eng = eng;
        avg = (math + c + eng) / 3;
        temp.Avg = avg;
        cset.push_back(c);
        mathset.push_back(math);
        engset.push_back(eng);
        avgset.push_back(avg);
        myset.push_back(temp);
    }
    sort(cset.rbegin(), cset.rend());
    sort(mathset.rbegin(), mathset.rend());
    sort(engset.rbegin(), engset.rend());
    sort(avgset.rbegin(), avgset.rend());

    vector<string> query(m);
    for (int i = 0; i < m; i++)
    {
        cin >> query[i];
    }
    vector<Rank>::iterator itr;
    for (int i = 0; i < m; i++)
    {

        id = query[i];
        itr = find_id(id);
        //int max = 0;
        char temp;
        if (itr < myset.end())
        {
            temp = find_top(itr);
            cout << (*itr).ran << ' ' << temp;
        }
        else
            cout << "N/A";
        if (i < m - 1)
            cout << endl;
    }
    int pause;
    cin >> pause;
}