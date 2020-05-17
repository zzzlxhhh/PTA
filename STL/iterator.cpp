#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <forward_list>
#include <algorithm>
#include <iterator>
using namespace std;
int main()
{
    /*
    1.random access iterator: vector deque array
    */
    vector<int> vec(10);
    vector<int>::iterator itr_v;
    itr_v = vec.begin();
    itr_v = itr_v + 5;
    itr_v = itr_v - 4;
    ++itr_v;
    --itr_v;
    /*faster than itr++ because ++itr dont need to return the
    previous value of itr*/

    /*
    2.bidirectional iterator: list set/multiset map/multimap
    */
    list<int>::iterator itr_l;
    ++itr_l;
    --itr_l; //itr=itr+5 is not allowed

    /*
    3.forward iterator: forward_list
    */
    forward_list<int>::iterator itr_fl;
    ++itr_fl; //--itr is not allowed

    //unordered containers provide "at least" forward iterators

    //every container has a iterator and a const_iterator
    set<int>::iterator it_s;
    set<int>::const_iterator cit_s;

    set<int> myset = {2, 4, 5, 1, 9};
    for (cit_s = myset.begin(); cit_s != myset.end(); ++cit_s)
    {
        cout << *cit_s << endl;
        //*citr=3 wont compile
    }
    //for_each(myset.cbegin(),myset.cend(),Myfunction) only in c++11
    advance(itr_v, 5);      //move itr forward 5 spots
    distance(itr_v, itr_v); //measure the distance between itr1 and itr2

    /*
     iterator adaptor (predefined iterator)
     1.insert iterator
     2.stream iterator
     3.reverse iterator
     4.move iterator
    */
    //1.insert iterator
    vector<int> v1 = {4, 5};
    vector<int> v2 = {12, 14, 16, 18};
    itr_v = find(v2.begin(), v2.end(), 16);
    insert_iterator<vector<int>> i_itr(v2, itr_v);
    copy(v1.begin(), v1.end(), //source
         i_itr);               //destination
    //other insert iterators: back_insert_iterator,front_insert_iterator

    //2.stream iterator
    vector<string> v3; //the default istream_iterator refers to the end of the stream
    copy(istream_iterator<string>(cin), istream_iterator<string>(),
         back_inserter(v3));
    copy(v3.begin(), v3.end(), ostream_iterator<string>(cout, ""));
    copy(istream_iterator<string>(cin), istream_iterator<string>(),
         ostream_iterator<string>(cout, " ")); //seperated by space

    //3.reverse iterator
    vector<int> v4 = {4, 5, 6, 7, 8};
    reverse_iterator<vector<int>::iterator> ritr;
    for (ritr = v4.rbegin(); ritr != v4.rend(); ritr++)
        cout << *ritr;

    /*
    algorithm 
    */
    vector<int> v5 = {4, 2, 5, 1, 3, 9};
    vector<int>::iterator itr = min_element(vec.begin(), vec.end());

    //note that algorithm process ranges in a half-open way [begin,end)
    sort(v5.begin(), itr);   //{ 2 4 5 1 3 9}
    reverse(itr, vec.end()); //{ 2 4 5 9 3 1 }

    //note
    vector<int> v6(3);
    copy(itr, v5.end(),
         v6.begin()); //v6 needs to have at least space for 3 elements

    v6.clear();
    //copy(itr,vec.end(),back_inserter(v6));
    //use back_insert_iterator: inserting instead of overwriting not efficient
    //not efficient because 
    v6.insert(vec.end(), itr, vec.end()); //efficient and safe
    //it is efficient because it only insert for one time


    //note: algorithm with function
    /*bool isOdd(int i)
    {
        return i%2;
    }
    itr=find_if(v6.begin(),v6.end(),isOdd);
    */


}