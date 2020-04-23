#include<iostream>
#include<vector>
#include<unordered_map>
#include<string>
#include<algorithm>
#include<iomanip>
using namespace std;
struct rec{
    int  D;
    int M;
    int H;
    int mm;
    string t;
    string name;
    bool online;
    int total;
};
vector<int> toll(24);
int Odaycos;
bool cmp(const rec a, const rec b)
{
    return a.total < b.total;
}
unordered_map<string,vector<rec>> cus_rec;
unordered_map<string,vector<pair<rec,rec>>> cus_bill;
bool cmpres(const pair<string,vector<pair<rec,rec>>> &a,const pair<string,vector<pair<rec,rec>>> &b)
{
    return a.first<b.first;
}
void printBill()
{
    vector<pair<string,vector<pair<rec,rec>>>> res_vec(cus_bill.begin(),cus_bill.end());
    sort(res_vec.begin(),res_vec.end(),cmpres);
    for(int i=0;i<res_vec.size();i++)
    {
        
        double Mcost=0;
        cout<<res_vec[i].first<<" "<<res_vec[i].second[0].second.t.substr(0,2)<<endl;
        for(int j=0;j<res_vec[i].second.size();j++)
        {
            int total_time=0;
            cout<<res_vec[i].second[j].first.t.substr(3)<<" "<<res_vec[i].second[j].second.t.substr(3);
            total_time=res_vec[i].second[j].second.total-res_vec[i].second[j].first.total;
            cout<<" "<<total_time<<" ";
            double cost=0;
            rec start=res_vec[i].second[j].first;
            rec end=res_vec[i].second[j].second;
            if(start.D==end.D)
            { 
               for(int p=start.H;p<=end.H;p++)
                if(p==start.H&&start.H==end.H) 
                cost+=(end.mm-start.mm)*toll[p];//同一天的同一小时下
                else if(p==start.H)
                cost+=(60-start.mm)*toll[p];
                else if(p<end.H) cost+=60*toll[p];
                else if(p==end.H) cost+=end.mm*toll[p];
            }
            else
            {
                /* code */
                cost+=(end.D-start.D-1)*Odaycos;
                for(int p=start.H;p<24;p++)
                if(p==start.H) cost+=(60-start.mm)*toll[p];
                else cost+=60*toll[p];

                for(int p=0;p<=end.H;p++)
                if(p==end.H) cost+=end.mm*toll[p];
                else cost+=60*toll[p];
                
            }
            
            printf("$%.2f",cost/100);
            cout<<endl;
            Mcost+=cost;//总开销
        }
        printf("Total amount: $%.2f\n",Mcost/100); 
    }
}
void mkbill()
{
    vector<pair<rec,rec>> temp_vec;
    unordered_map<string, vector<rec>>::iterator rec_itr = cus_rec.begin();
    for(;rec_itr!=cus_rec.end();rec_itr++)
    {
        sort(rec_itr->second.begin(),rec_itr->second.end(),cmp);
        vector<rec>::iterator vec_itr=rec_itr->second.begin();
        unordered_map<string, vector<pair<rec,rec>>>::iterator bill_itr = cus_bill.begin();
        for(;vec_itr<(rec_itr->second.end())-1;)//注意越界问题
        {
            if(rec_itr->second.size()==1) break;//可放在外层
            if(vec_itr->online==true&&(vec_itr+1)->online==false)
                {
                    bill_itr=cus_bill.find(vec_itr->name); //忘掉了！！！
                    if(bill_itr!=cus_bill.end())
                    bill_itr->second.push_back(make_pair(*vec_itr,*(vec_itr+1)));
                    else
                    {
                        temp_vec.push_back(make_pair(*vec_itr,*(vec_itr+1)));
                        cus_bill.insert(make_pair(vec_itr->name,temp_vec));
                        temp_vec.clear();
                    }
                    vec_itr+=2;
                    
                }
            else
                vec_itr++;            

        }
    }
}
void get_parse(rec &rec_in)
{
    string buf;
    cin>>rec_in.name;
    char *cha;
    cin>>buf;
    rec_in.t=buf;
    cha=(char*)buf.data();
    sscanf(cha,"%d:%d:%d:%d",&rec_in.M, &rec_in.D, &rec_in.H, &rec_in.mm);
    rec_in.total=(rec_in.D*24+rec_in.H)*60+rec_in.mm;
    cin>>buf;
    if(buf=="on-line")
    rec_in.online=1;
    else rec_in.online=0;
    return ;
}

int main() {
    Odaycos=0;
    for(int i=0;i<24;i++)
        {cin>>toll[i];Odaycos+=toll[i]*60;}
    int rec_num;
    cin>>rec_num;
    vector<rec> temp_vec;
    for(int i=0;i<rec_num;i++)
    {
        
        rec rec_in;
        get_parse(rec_in);
        unordered_map<string, vector<rec>>::iterator itr = cus_rec.begin();
        itr=cus_rec.find(rec_in.name);
        if(itr!=cus_rec.end())
        itr->second.push_back(rec_in);
        else
        {
        temp_vec.push_back(rec_in);
        cus_rec.insert(make_pair(rec_in.name,temp_vec));
        }
        temp_vec.clear();
    }
    mkbill();
    printBill();
    return 0;

}