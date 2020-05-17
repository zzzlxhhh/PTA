#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
using namespace std;
class AddValue
{
    int val;

public:
    AddValue(int j) : val(j) {}
    void operator()(int i)
    {
        cout << i + val << endl;
    }
};
int main()
{
    vector<int> vec = {2, 3, 4, 5};
    int x = 2;
    for_each(vec.begin(), vec.end(), AddValue(x));
    cin >> x;
    //buit in functors
    /*
    less greater greater_equal less_equal not_equal_to
    logical_and logical_not logical_or multiplies 
    minus plus divide modules negate
    */
    /*
   why dow we need functors in STL
   */
    set<int> myset = {3, 1, 25, 7, 12};
    //same as
    set<int, less<int>> myset1 = {3, 1, 25, 7, 12};
    //least significant number
    class Lsb_less
    {
    public:
        bool operator()(int x, int y)
        {
            return (x % 10) < (y % 10);
        }
    };
    set<int, Lsb_less> myset2 = {3, 1, 25, 7, 12};
}