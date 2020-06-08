#include<iostream>
#include<stack>
#define maxsize 100
using namespace std;
struct Node   //静态链表,使用该结构 输入顺序可以任意 不以根节点开始输入
{
	int left;//-1表示为空
	int right;
	char elem;
};
Node arr1[maxsize];
Node arr2[maxsize];
int buildTree(Node arr[])//返回根节点
{
	int n;
	cin>>n;//输入节点个数
	int check[maxsize];
	for(int i=0; i<n; i++)
		check[i]=0;
	char l,r;
	for(int i=0; i<n; i++)
	{
		cin>>arr[i].elem>>l>>r;
		if(l!='-')
		{
			arr[i].left=l-'0';
			check[l-'0']=1;//表明有节点指向节点l，由父指向儿
		}//而根节点无父节点，可根据check判断
		else
			arr[i].left=-1;//左子树为空
		if(r!='-')
		{
			arr[i].right=r-'0';
			check[r-'0']=1;//表明有节点指向节点l
		}
		else
			arr[i].right=-1;//左子树为空

	}
	int i;
	for(i=0; i<n; i++)
		if(!check[i]) break;
	return i;
}
int isomophic(int r1,int r2)
{
	if(r1==-1&&r2==-1) return 1;//根都为空
	else if(r1==-1||r2==-1) return 0;//根一个空一个不空
	if(arr1[r1].elem!=arr2[r2].elem) return 0;//根都不为空 则比较元素
	if(arr1[r1].left==-1&&arr2[r2].left==-1)//左子树都为空 比较右子树
		return isomophic(arr1[r1].right,arr2[r2].right);
	//左儿子都不为空，且相等则继续 不需要swap比较下一层
	if(arr1[r1].left!=-1&&arr2[r2].left!=-1
	        &&arr1[arr1[r1].left].elem==arr2[arr2[r2].left].elem)
		return isomophic(arr1[r1].left,arr2[r2].left)&&
		       isomophic(arr1[r1].right,arr2[r2].right);
	//
	else return isomophic(arr1[r1].left,arr2[r2].right)&&
		            isomophic(arr1[r1].right,arr2[r2].left);
}
void midtra(Node arr[],int r)
{
	if(r!=-1)
	{
		midtra(arr,arr[r].left);
		cout<<arr[r].elem<<"->";
		midtra(arr,arr[r].right);
	}
}
void midtraQ(Node arr[],int r)
{
	stack<int> s;
	//此处为空的判断一定是-1
	while(r!=-1||!s.empty())
	{
		while(r!=-1)
		{
			s.push(r);
			r=arr[r].left;
		}
		if(!s.empty())
		{
			int t=s.top();
			s.pop();
			cout<<t<<"->";
			r=arr[t].right;
		}
	}
}
int main()
{
	int root1,root2;
	cout<<"first tree:"<<endl;
	root1=buildTree(arr1);
	cout<<"seconf tree:"<<endl;
	root2=buildTree(arr2);
	if(isomophic(root1,root2))
		cout<<"isomophic"<<endl;
	cout<<"recursive tra:";
	cout<<'\n';
	cout<<"arr1";
	midtra(arr1,root1);
	cout<<endl;
	cout<<"arr2";
	midtra(arr2,root2);
	 
	cout<<"stack tra:";
	cout<<'\n';
	cout<<"arr1";
	midtraQ(arr1,root1);
	cout<<endl;
	cout<<"arr2";
	midtraQ(arr2,root2); 
	
}