#include<iostream>
#include<map>
#include<stack>
#include<vector>
#include<queue>
#include<cstring>
using namespace std;
//算法类似于BFS
map<int,vector<int>> road;
int main()
{
    int v,e;
    cin>>v>>e;//input vertex and edge
    map<int,vector<int>>::iterator itr;
    vector<int> tmp_vec;
    int tmpi,tmpj;//assume one-dierection 
    for(int i=0;i<e;i++)
    {
        cin>>tmpi>>tmpj;
        itr=road.find(tmpi);
        if(itr!=road.end())
        {
            itr->second.push_back(tmpj);
        }
        else
        {
            tmp_vec.push_back(tmpj);
            road.insert(make_pair(tmpi,tmp_vec));
            tmp_vec.clear();
        }        
    }
    //search for shortest path
    queue<int> q;
    vector<int> path(v+1);
    vector<int> dist(v+1);//到起始点的距离
    fill(path.begin(),path.end(),-1);
    fill(dist.begin(),dist.end(),-1);
    int start;
    cin>>start;//start point
    dist[start]=0;//initialize
    q.push(start);
    int vtmp;
    while(!q.empty())
    {
        vtmp=q.front();
        q.pop();
        itr=road.find(vtmp);
        for(int i=0;i<itr->second.size();i++)//邻接点遍历
        {
            if(dist[itr->second[i]]==-1)
            {
                q.push(itr->second[i]);
                dist[itr->second[i]]=dist[itr->first]+1;
                path[itr->second[i]]=itr->first;
            } 
        }    
    }
    int end,tmp;
    cin>>end;tmp=end;//end point
    stack<int> result;
    while(path[tmp]!=start)
    {
        result.push(path[tmp]);
        tmp=path[tmp];
    }
    cout<<start<<"->";
    while(!result.empty())
    {
        cout<<result.top()<<"->";
        result.pop();
    }
    cout<<end;
}