#include <iostream>
#include <vector>
#include <set>
#include <stack>
using namespace std;
int N, M, K;
bool visited[1010];
void dfs(int i, vector<int> *road)
{
    visited[i] = true;
    vector<int>::iterator itr;
    for (itr = road[i].begin(); itr != road[i].end(); itr++)
    {
        if (!visited[*itr])
        {
            //visited[*itr] = true;
            dfs(*itr, road);
        }
    }
}
int main()
{
    scanf("%d%d%d", &N, &M, &K);
    vector<int> road[N + 1];
    int t1, t2;
    for (int i = 0; i < M; i++)
    {
        cin >> t1 >> t2;
        road[t2].push_back(t1);
        road[t1].push_back(t2);
    }
    //int *checklist = (int *)malloc(sizeof(int) * K);

    int check, count;
    for (int i = 0; i < K; i++)
    {
        count = 0;
        fill(visited, visited + N + 10, false);
        cin >> check;
        visited[check] = true;
        for (int i = 1; i < N + 1; i++)
        {
            if (!visited[i])
            {
                dfs(i, road);
                ++count;
            }
        }
        printf("%d\n", count - 1);
    }
    int pause;
    cin >> pause;
}