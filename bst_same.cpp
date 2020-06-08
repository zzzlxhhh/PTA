#include <iostream>
#include <vector>
#include <stack>
#define maxsize 100
struct Node
{
    int elem;
    Node *left;
    Node *right;
    int flag;
};
using namespace std;
Node *Insert(Node *bst, int x)
{
    if (!bst)
    { //原始树为空 必要的判断，不然递归无法正常建树
        bst = (Node *)malloc(sizeof(Node));
        bst->elem = x;
        bst->left = bst->right = NULL;
        bst->flag = 0;
    }
    else
    {
        if (x < bst->elem) //插入左子树
            bst->left = Insert(bst->left, x);
        else if (x > bst->elem)
            bst->right = Insert(bst->right, x);
        //else x已经存在 什么都不用做
    }
    return bst;
}
void Reset(Node *bst)
{
    if(bst->left) Reset(bst->left);
    if(bst->right) Reset(bst->right);
    bst->flag = 0;
}
void freeTree(Node *bst)
{
    if(bst->left) freeTree(bst->left);
    if(bst->right) freeTree(bst->right);
    free(bst);
}
int check(Node *t, int tmp)
{
    if (t->flag) //如果路径上经过的节点之前均出现过，则一致
    {
        if (tmp < t->elem)
            return check(t->left, tmp);
        else if (tmp > t->elem)
            return check(t->right, tmp);
        else
            return 0; //表明重复 返回0
    }
    else //之前未出现过，可能就是搜索节点，也可能是不一致的节点
    {
        if (tmp == t->elem)
        {
            t->flag = 1; //就是当前搜索节点
            return 1;
        }
        else
            return 0;
    }
}
int Judge(Node *t, int n) //要设立flag ，不能序列每输入完就结束返回
{
    int tmp;
    int flag = 0; //0表示一致
    //加快程序的速度先进行判断,减少check的次数
    cin >> tmp;
    if (tmp != t->elem)
        flag = 1;
    else
        t->flag = 1;
    for (int i = 1; i < n; i++)
    {
        cin >> tmp;
        if ((!flag) && !check(t, n))
            flag = 1;
    }
    if (flag)
        return 0;
    else
        return 1;
}
int main()
{
    Node *bst = NULL;
    vector<int> v;
    int n, tmp, l;
    cin >> n >> l;//一棵树n个节点 判断l组序列
    //int *arr=(int*)malloc(n*sizeof(int));

    for (int i = 0; i < n; i++)
    {
        cin >> tmp;
        bst = Insert(bst, tmp);
    }
    for (int i = 0; i < l; i++)
    {
        if (Judge(bst, n))
            cout << "yes" << endl;
        else
            cout << "No\n";
        Reset(bst);
    }
    freeTree(bst);
    //cout << bst->elem << endl;
}