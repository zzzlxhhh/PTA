#include<iostream>
#include<map>
#include<stack>
#include<vector>
#include<queue>
#include<cstring>
using namespace std;
//#define MAX_VAL 99999;
/*
令S={源点s+已经确定了最短路径的顶点vi}
对人以未收录的顶点v，定义dist[v]为s到v的最短路径长度，但该路径仅经过s中的
顶点。即路径{s->(vi e S)->v}的最小长度
*/
//即不是bfs也不是dfs
map<int,vector<pair<int,int>>> road;
int main()
{
    int v,e;
    cin>>v>>e;//input vertex and edge and weight
    map<int,vector<pair<int,int>>>::iterator itr;
    vector<pair<int,int>> tmp_vec;
    int tmpi,tmpj,tmpw;//assume one-dierection 
    for(int i=0;i<e;i++)
    {
        cin>>tmpi>>tmpj>>tmpw;
        itr=road.find(tmpi);
        if(itr!=road.end())
        {
            itr->second.push_back(make_pair(tmpj,tmpw));
        }
        else
        {
            tmp_vec.push_back(make_pair(tmpj,tmpw));
            road.insert(make_pair(tmpi,tmp_vec));
            tmp_vec.clear();
        }        
    }
    //search for shortest path
    vector<int> path(v+1);
    vector<int> dist(v+1);//attention for definition of dist
    fill(path.begin(),path.end(),-1);
    fill(dist.begin(),dist.end(),999999);
    
    int start,end;
    cin>>start>>end;//start point
    
    vector<pair<int,int>>::iterator vec_itr;
    itr=road.find(start);
    vec_itr=itr->second.begin(); //初始化起点邻接点
    for(int i=0;i<itr->second.size();i++)//attention !!!!!! initialize!!!!
        {
            path[(vec_itr+i)->first]=start;
            dist[(vec_itr+i)->first]=(vec_itr+i)->second; 
        }   //start point's neighbor
    dist[start]=0;

    vector<int> collected(v+1);
    fill(collected.begin(),collected.end(),0);
    int tmp;
    int min;
    while(1)
    {
        tmp=-1;
        min=999999;
       for(int i=1;i<v+1;i++)//从点1开始
       if(dist[i]<min&&collected[i]==0){tmp=i;min=dist[i];} 
        //未收录的最小的点的tmp
        if(tmp==-1) break;
        collected[tmp]=1;
        itr=road.find(tmp);
        vec_itr=itr->second.begin();
        //tmp的邻接点
        for(;vec_itr<itr->second.end();vec_itr++)
        {
            int n=vec_itr->first;//邻接点下标
            if(collected[n]==0)
            {
                if(dist[tmp]+vec_itr->second<dist[n])
                {
                    dist[n]=dist[tmp]+vec_itr->second;
                    path[n]=tmp;
                }
            }
        }    
    }
    tmp=end;
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