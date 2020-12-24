#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <string>
#include <set>
#include <vector>
#include <map>
using namespace std;

class Project{ //项目
public:
    string key; //项目的key
    int value_num; //项目key对应的value的编号
    int index; //所处位置
    int type; //1为移进项 2为归约项
    Project(string a="", int b=0, int c=0, int d=1){
        this->key = a;
        this->value_num = b;
        this->index = c;
        this->type = d;
    }
    bool operator == (const Project &other){
        return (this->key==other.key)&&(this->value_num==other.value_num)&&(this->index==other.index)&&(this->type==other.type);
    }
};

class Grammar{
private:
    map<string, vector<vector<string>>> m; //记录文法的每一条产生式，通过键值对的方式
    string start;  //记录开始符号
    set<string> ntSet; //终结符集合
    map<string, set<string>> m_first; //记录每一个非终结符号对应的first集合
    map<string, set<string>> m_follow; //记录每一个非终结符对应的follow集合
    vector<vector<Project>> DFA_nodes; //记录DFA结点
    map<int, map<string, int>> forwards; //记录移进关系
    map<int, map<string, int>> backs; //记录归约关系
    bool isSLR1; //是否为SLR(1)文法
    string reason; //不是SLR(1)文法的原因
public:
    Grammar(string inputString=""); //构造函数(完成初始化操作)
    string get_start(); //得到开始符号
    set<string> get_ntSet();  //得到终结符集
    set<string> get_first(string key); //得到非终结符key的first集合
    set<string> get_follow(string key);//得到非终结符key的follow集合
    string show(); //展示文法
    string parse(string sentence); //分析
private:
    void first_follow_all(); //求所有非终结符的first和follow函数
    set<string> first(vector<string> value); //求value序列的first集合
    void node_relationship(); //求DFA图，顺便把归约关系也求了，顺便判断是否为SLR1
    void extend(int k); //扩展结点k
};

#endif