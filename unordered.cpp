#include <iostream>
#include <unordered_set>
#include <vector>
#include <unordered_map>
using namespace std;
void foo(const unordered_map<char, string> &m)
{
    //m['s']="sunday"//error     const

    /*it also need to notice that trying to read is also
    considered to be unallowed by compiler,it will preassume
    that you are trying to write because of the "[]"
    that is:
    cout<<m['s']<<endl;
    */
    auto itr = m.find('s');
    if (itr != m.end())
        cout << (*itr).second << endl;
}
int main()
{

    /*unordered set & multiset
 unordered map & multimap:
 implementation:hash table
 if you have a effective hash functor,it would be fast to find a element
 only takes constant time o(1)
*/
    unordered_set<string> myset = {"red", "green", "blue"};
    unordered_set<string>::const_iterator it = myset.find("green");
    // end()refers to undefined behavior,so it's important to check
    if (it != myset.end()) //to check
        cout << *it << endl;
    myset.insert("yellow");

    //convenient use
    vector<string> vec = {"purple", "pink"};
    myset.insert(vec.begin(), vec.end());

    //hash table specific API
    cout << "load_factor=" << myset.load_factor() << endl;
    string x = "red";
    cout << x << " is in bucket#" << myset.bucket(x) << endl;
    cout << "Total bucket:" << myset.bucket_count() << endl;

    /*
 unordered muliset: unordered set that allows duplicated elements
 unordered map: unordered set of pairs
 unordered multimap: unordered map that allows duplicated keys
 
 hash collision results in performance degrade
 worst situation: all the elements stored in a single bucket
 leading: O(1)->O(n)
*/

    /*properties:
 1.fastest search&insert at any place:O(1)
 associative container takes O(log(n))
 vector,deque takes O(n)
 list takes O(1) to insert,O(n) to search
 2.unordered set/multiset: element value cannot be changed
   unordered map/multimap: element key cannot be changed
*/

    /*associative array
 could be implemented with map or unordered
*/
    unordered_map<char, string> day = {{'s', "sunday"}, {'m', "monday"}};
    cout << day['s'] << endl;    // no range check
    cout << day.at('s') << endl; //has range check

    vector<int> v = {1, 2, 3};
    v[5] = 5; //compiling error

    day['w'] = "wednesday";               //inserting {'w',"wednesday"}
    day.insert(make_pair('f', "friday")); //inserting
    day.insert(make_pair('m', "MONDAY")); //fail to modify
    day['m'] = "MONDAY";                  //succeed to modify
}