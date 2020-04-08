#include <iostream>
#include <string>
#include <math.h>
#include <vector>
using namespace std;
struct str
{
    string num;
    int radix;
};
vector<str> vec;
bool isPrime(int n)
{
    float n_sq = floor(sqrt(n));
    if (n == 2 || n == 3)
        return true;
    if (n % 6 != 1 && n % 6 != 5||n==1)
        return false;
    for (int i = 5; i < n_sq; i += 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    return true;
}
bool toDex(str temp)
{
    int res1 = 0;
    int res2 = 0;
    int size = temp.num.size();
 //   for (int i = 0; i < size; i++)
   //     res1 = temp.num[i] - '0' + res1 * temp.radix;
    res1=stoi(temp.num,0,10);
    if (!isPrime(res1))
        return false;
    string t = "";
    while (res1)
    {
        t += res1 % temp.radix + '0';
        res1 = res1 / temp.radix;
    }
    res2 = stoi(t, 0, temp.radix);
    if (isPrime(res2))
        return true;
    else
    {
        return false;
    }
}
int main()
{
    int n;
    while (1)
    {
        str s;
        cin >> s.num;
        if (s.num[0] == '-')
            break;
        cin >> s.radix;
        vec.push_back(s);
    }
    for (int i = 0; i < vec.size(); i++)
    {
        if (toDex(vec[i]))
            cout << "Yes" << endl;
        else
            cout << "No" << endl;
    }
    cin >> n;
}
