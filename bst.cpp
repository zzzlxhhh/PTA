#include<iostream>
#include<vector>
#include<stack>
#define maxsize 100
struct Node {
	int elem;
	Node *left;
	Node *right;
};
using namespace std;
Node* Insert(Node *bst,int x)
{
	if(!bst) {//原始树为空 必要的判断，不然递归无法正常建树
	bst=(Node *)malloc(sizeof(Node));
	bst->elem=x;
    bst->left=bst->right=NULL;
	}
    else
    {
        if(x<bst->elem)//插入左子树
        bst->left=Insert(bst->left,x);
        else if(x>bst->elem)
        bst->right=Insert(bst->right,x);
        //else x已经存在 什么都不用做
    }
    return bst;
}
/*递归实现
Node* find(Node *bst,int x)
{
    if(!bst) return NULL;
    if(x>bst->elem) return find(bst->right,x);
    else if(x<bst->elem) return find(bst->left,x);
    else return bst;
}
*/
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
Node* del(Node *bst,int x)//返回删除节点后新的儿子节点
{
    //树为空，必要的判断
    if(!bst) cout<<"未找到删除的元素";
    else if(x<bst->elem)//在左子树中删除
        bst->left=del(bst->left,x);
    else if(x>bst->elem)
        bst->right=del(bst->right,x);
    else if(bst->left&&bst->right)
    {
        Node *tmp=findmin(bst->right);
        bst->elem=tmp->elem;//只替换掉原来节点的值
        bst->right=del(bst->right,bst->elem);//在右子树中删除该点
        //不用担心其内存释放的问题，右子树中的最小值的度是0/1
    }
    else//要删除的目标节点只有一个儿子，或者没有儿子
    {
        Node *tmp=bst;
        if(!bst->left)//!!!!!此处的条件是左子树为空，对应着有右孩子或者没有孩子
            bst=bst->right;//故不用单独再给没有孩子设置一个判断
        else if( !bst->right)
            bst=bst->left;
        free(tmp);
    }
    return bst;
}
void midtra(Node* bst)
{
    if(bst)
    {
        midtra(bst->left);
        cout<<bst->elem<<"->";
        midtra(bst->right);
    }
}
void midtraS(Node* bst)
{
    stack<Node> s;
    while(bst||!s.empty())
    {
        while(bst)
        {
            s.push(*bst);
            bst=bst->left;            
        }
        if(!s.empty())
        {
            cout<<s.top().elem<<"->";          
            bst=s.top().right;
            s.pop();
        }
    }
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
        bst=Insert(bst,tmp);
    }
    cout<<bst->elem<<endl;
    cout<<find(bst,10)->elem<<endl;
//    cout<<"min:"<<findmin(bst)->elem<<endl;
    cout<<"max:"<<findmax(bst)->elem<<endl;
    cout<<"tra :";
    midtra(bst);
    cout<<endl;
    cout<<"traS:";
    midtraS(bst);
    cout<<endl;
    del(bst,11);
    cout<<"tra after del 9:";
    midtra(bst);
    cout<<endl;
}