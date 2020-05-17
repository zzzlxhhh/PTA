#include<iostream>
#include<set>
#include<vector>
#include<map>
#include<unordered_set>
#include<unordered_map>
using namespace std;
int main()
{
/*associative containers*/

/*set  sort by value
-no duplicated
*/
set<int> myset;
myset.insert(3);
myset.insert(1);
myset.insert(7);//log(n)
set<int>::iterator it;
it=myset.find(8); //log(n)

pair<set<int>::iterator,bool> ret;
ret=myset.insert(3);//no element inserted
if(ret.second==false)
    it=ret.first;

myset.insert(it,9);//1 3 7 9
//hint to make insertion cost less time
myset.erase(it); //1 7 9
myset.erase(7);//1 9
//none of this sequence container provide this kind of erase

//mutiset allows duplicated items
//multiset<int> myset;
//set or multiset: value of the elements cannot be modified
// *it=10 is not allowed which is read-only

/* set & multiset
property:
1. fast search log(n)
2. traversing is slow(compared to vector and deque)
3. no random access, no [] operator
*/

/*map (key to value) sorted by key
* no duplicated key
*
*/
map<char,int> mymap;
mymap.insert(pair<char,int>('a',100));
mymap.insert(make_pair('z',200));

map<char,int>::iterator itr=mymap.begin();
mymap.insert(itr,pair<char,int>('b',300));

itr=mymap.find('r');
for(itr=mymap.begin();itr!=mymap.end();itr++)
cout<<(*itr).first<<"=>"<<(*itr).second<<endl;
//multimap allows duplicated keys
/*
map & multimap keys cannot be modified
type of *itr: pair<const char,int>
*/
int pause;
cin>>pause;



}