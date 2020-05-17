#include<iostream>
#include<vector>
#include<algorithm>
#include<string.h>
using namespace std;
vector<int> vec;
int  main() {
    int V,N,tmp;
    cin>>V>>N;//V种类 N面值
    for(int i=0;i<V;i++)
    {
        cin>>tmp;
        vec.push_back(tmp);
    }
    long long int res[1002];
    memset(res,0,1001);
    res[0]=1;
    sort(vec.begin(),vec.end());
    for(int i=0;i<V;i++)
        for(int j=vec[i];j<=N;j++)
            res[j]+=res[j-vec[i]];
    cout<<res[N];
    cin>>V;
    return 0;
}