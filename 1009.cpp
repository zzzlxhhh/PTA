#include<iostream>
#include<algorithm>
#include<vector>
#include<deque>
#include<stack>
#include<iomanip>
using namespace std;
struct node {
	int exponents;
	double coefficients;
};
bool cmp(node a,node b)
{//升序
    return a.exponents<b.exponents; 
}
int main()
{
	vector<node> p1;
	vector<node> p2;
	deque<node> p;
	int count1, count2;
	
	cin >> count1;
	node temp;
	for (int i = 0; i < count1; i++)
	{
		cin >> temp.exponents;
		cin >> temp.coefficients;
		p1.push_back(temp);//队列尾部
	}
	cin >> count2;
	for (int i = 0; i < count2; i++)
	{
		cin >> temp.exponents;
		cin >> temp.coefficients;
		p2.push_back(temp);
	}

    for(int i=0;i<p1.size();i++)
    {
        for(int j=0;j<p2.size();j++)
        {
            temp.coefficients=p1[i].coefficients*p2[j].coefficients;
            temp.exponents=p1[i].exponents+p2[j].exponents;
            p.push_back(temp);
        }
    }

    sort(p.begin(),p.end(),cmp);
    
    stack<node> st;
    deque<node>::iterator itr;
    while(!p.empty())
    {
        itr=p.begin();
        if((*itr).exponents==(*(itr+1)).exponents)
        {
            temp.coefficients=(*itr).coefficients+(*(itr+1)).coefficients;
            temp.exponents=(*itr).exponents;
            p.pop_front();
            p.pop_front();
            if(temp.coefficients!=0)
            p.push_front(temp);
        }
        else 
        {
            st.push(*itr);
            p.pop_front();
        }
    }
    
	cout << st.size();
	while (st.size() != 0)
	{
		cout << " " << fixed <<setprecision(1)<< st.top().exponents << " " << st.top().coefficients;
		st.pop();
	}
    int count;
	cin >> count;
}
