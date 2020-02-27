#include <iostream>
#include <string>
using namespace std;

long long int stringToNum(string n1, int radix)
{
    long long int val1 = 0;
    for (int i = 0; i < n1.size(); i++)
    {
        if (n1[i] >= 'a')
            val1 = val1 * radix + n1[i] - 'a' + 10;
        else
            val1 = val1 * radix + n1[i] - 48;
    }
    return val1;
}
int func(int s, string str, int tarval)
{
    long long int val = stringToNum(str, s);
    long long int l, r;
    l = s;
    r = tarval>l?tarval:l;
    long long int mid;
        while (r>=l)
        {
            mid = (l + r) / 2;
            val = stringToNum(str, mid);
            if (val > tarval || val < 0)
            {
                r = mid - 1;
            }
            else if (val < tarval)
            {
                l = mid + 1;
            }
            else if (val == tarval)
                return mid;
            
        }
    return -1;
}
int main()
{
    string n1, n2;
    int temptag;
    long long int radix1, radix2;
    long long int val1 = 0, val2 = 0;
    bool r = true; //已知1的基数
    cin >> n1 >> n2 >> temptag;

    if (temptag == 1)
    {
        cin >> radix1;
    }
    else
    {
        cin >> radix2;
        r = false;
    }

    if (r)
    {
        int max = 0;
        int tmp = 0;
        for (int i = 0; i < n2.size(); i++)
        {
            if (n2[i] >= 'a')
                tmp = n2[i] - 'a' + 10;
            else
                tmp = n2[i] - 48;
            if (tmp > max)
                max = tmp;
        }
        //radix2 = max + 1;
        radix2 = (max + 1)>1? max+1:max+2;
        val1 = stringToNum(n1, radix1);
        val2 = stringToNum(n2, radix2);
    }
    else
    {
        int max = 0;
        int tmp = 0;
        for (int i = 0; i < n1.size(); i++)
        {
            if (n1[i] >= 'a')
                tmp = n1[i] - 'a' + 10;
            else
                tmp = n1[i] - 48;
            if (tmp > max)
                max = tmp;
        }
        radix1 = (max + 1)>1? max+1:max+2;
        val1 = stringToNum(n1, radix1);
        val2 = stringToNum(n2, radix2);
    }
    int result;
    if (r)
    {
        result = func(radix2, n2, val1);
    }
    else
    {
        result = func(radix1, n1, val2);
    }

    if (result != -1)
        cout << result;
    else
    {
        cout << "Impossible";
    }
   // int pause;
    //cin >> pause;
}