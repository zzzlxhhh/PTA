#include <iostream>
#include <vector>
#include <stack>
#define maxsize 100
struct Node
{
	int elem;
	Node *left;
	Node *right;
	int height;
};
using namespace std;
int max(int a, int b)
{
	return a > b ? a : b;
}
int getheight(Node *t)
{
	int HL,HR,H;
	if(t)
	{
		HL=getheight(t->left);
		HR=getheight(t->right);
		H=max(HL,HR);
		return H+1;
	}
	return 0;
}
Node *RR(Node *t) //右子树的右边
{
	Node *b = t->right;
	t->right = b->left;
	b->left = t;
	t->height = max(getheight(t->left), getheight(t->right)) + 1;
	b->height = max(t->height, getheight(b->right)) + 1;
	return b; //!!返回值新的树的根节点
}
Node *LL(Node *t) //左子树的左边
{
	Node *b = t->left;
	t->left = b->right;
	b->right = t;
	t->height = max(getheight(t->left), getheight(t->right)) + 1;
	b->height = max(t->height, getheight(b->right)) + 1;
	return b;
}
Node *LR(Node *t) //左子树的右边
{
	//A必须右一个左节点B,而B必须有一个有子节点C
	//将A、B与C做两次单旋，返回新的节点
	t->left = RR(t->left);
	return LL(t);
}
Node *RL(Node *t)
{
	t->left = LL(t->right);
	return RR(t);
}
Node *insert(Node *t, int x)
{
	if (!t)
	{
		t = (Node *)malloc(sizeof(Node));
		t->elem = x;
		t->height = 0;
		t->left = t->right = NULL;
	}
	else if (x < t->elem)
	{ //插入左子树
		t->left = insert(t->left, x);
		//if(t->height==2)!!不能这样判断，height还没有更新
		//往左边插入，故左边的height增加，判断高度为+2
		if (getheight(t->left) - getheight(t->right) == 2)
		{
			if (x < t->left->elem) //插入左子树的左边 故为LL
				t = LL(t);
			else //左子树的右边
				t = LR(t);
		}
	}
	else if(x>t->elem)
	{
		t->right=insert(t->right,x);
		if(getheight(t->left)-getheight(t->right)==-2)
		{
			if(x>t->right->elem)
				t=RR(t);
			else
				t=RL(t);
		}
	}
	//x==t->elem 无需插入
	//更新树高度
	t->height=max(getheight(t->left),getheight(t->right))+1;
	return t;
}
Node* find(Node *bst,int x)
{
    while(bst)
    {
        if(x>bst->elem)
            bst=bst->right;
        else if(x<bst->elem)
            bst=bst->left;
        else return bst;
    }
    return NULL;
}
Node* findmax(Node *root)
{
    if(root)
    while (root->right) root=root->right;
    return root;
}
Node* findmin(Node *root)
{
    if(root)
    while(root->left) root=root->left;
    return root;
}
int main() 
{
    Node *bst=NULL;
    vector<int> v;
    int n,tmp;
    cin>>n;
    //int *arr=(int*)malloc(n*sizeof(int));
    for(int i=0;i<n;i++)
    {
        cin>>tmp;
        bst=insert(bst,tmp);
    }
    cout<<bst->elem<<endl;
    cout<<find(bst,10)->elem<<" height:"<<find(bst,10)->height<<endl;
	cout<<"min->right:"<<findmin(bst)->right->elem<<endl;
	return 0;
}