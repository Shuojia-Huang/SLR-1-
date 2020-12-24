#include "grammar.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

Grammar::Grammar(string inputString){
    /*
    先用字符串分割的方式，找到每一个产生式子串
    再采用字符串流的方法，因为可以读取时可以按照空格、换行等等来分割
    */
    //分割出每一个产生式并存于可变数组中
    vector<string> v;
    int from = 0;
    int len = 0;
    int i;
    for(i=0; i<inputString.length(); ++i){
        if(inputString[i] == '\n'){
            len = i - from;
            v.push_back(inputString.substr(from, len));
            from = i + 1;
        }
    }
    v.push_back(inputString.substr(from));
    //文法检查扩充
    stringstream ss(v[0]);
    string s;
    ss >> s;
    if(v[0].find('|') != string::npos){
        v.insert(v.begin(), string(s+"'"+" -> "+s));
        s += "'";
    }
    this->start = s;
    //处理每一个产生式变成键值对
    for(auto &x: v){
        string key;
        vector<vector<string>> values;
        string a;
        vector<string> value;
        stringstream ss(x+" $");
        ss >> key;
        ss >> a;
        while(1){
            ss >> a;
            if(a == "$"){
                break;
            }
            if(a == "|"){
                values.push_back(value);
                value.clear();
                continue;
            }
            int from = 0;
            int len = 0;
            int i;
            for(i=0; i<a.length(); ++i){
                if(a[i] == '(' || a[i] == ')'){
                    len = i-from;
                    if(len != 0){
                        value.push_back(a.substr(from, len));
                    }
                    from = i + 1;
                    value.push_back(string(1, a[i]));
                }
            }
            if(from != i){
                value.push_back(a.substr(from));
            }
        }
        values.push_back(value);
        this->m[key] = values;
    }

    /*求非终结符集*/
    for(auto it = m.begin(); it != m.end(); ++it){
        this->ntSet.insert(it->first);
    }

    /*求所有终结符的follow集合和follow集合*/
    this->first_follow_all();

    /*求LR(0)DFA、判断是否为SLR(1)文法、SLR(1)表格*/
    this->node_relationship();
    
}

string Grammar::get_start(){
    return this->start;
}

set<string> Grammar::get_ntSet(){
    return this->ntSet;
}

string Grammar::show(){
    /*
    定义字符流，其目的是可以用类似于文件流的方法将展示结果置于字符流中，通过str()函数即可转为字符串形式outputStr，
    如果我们想在终端输出，直接用cout << outputStr 即可。
    如果我们想在QT的窗口输出，传递outputStr即可。
    */
    stringstream ss;
    //输入所有终结符的first集合和follow集合
    ss << "所有非终结符first集合为:\n";
    for(auto &nt: this->ntSet){
        ss << nt << ": ";
        for(auto &x: this->get_first(nt)){
            ss << x << " ";
        }
        ss << "\n";
    }
    ss << "follow集合为:\n";
    for(auto &nt: this->ntSet){
        ss << nt << ": ";
        for(auto &x: this->get_follow(nt)){
            ss << x << " ";
        }
        ss << "\n";
    }
    ss << "\nDFA图为：\n";
    //输出DFA图
    ss << "DFA结点编号和对应的项目集为：\n";
    for(int i=0; i<this->DFA_nodes.size(); ++i){
        ss << "编号" << i << ":\n";
        for(auto &p: this->DFA_nodes[i]){
            ss << p.key << " -> ";
            for(int j=0; j<this->m[p.key][p.value_num].size(); ++j){
                if(j == p.index){
                    ss << ".";
                }
                ss << this->m[p.key][p.value_num][j] << " ";
            }
            if(p.index == this->m[p.key][p.value_num].size()){
                ss << ".";
            }
            ss << '\n';
        }
    }
    ss << "DFA结点之间的有向边为(采用三元组的方式):\n";
    for(auto &x: this->forwards){
        int from = x.first;
        for(auto &y: x.second){
            string t = y.first;
            int to = y.second;
            ss << "(" << from << ", " << t << ", " << to << ")  ";
        }
        ss << '\n';
    }
    //输出是否为SLR(1)文法，如不是再输出原因
    if(this->isSLR1){
        ss << "\n该文法为SLR(1)文法\n";
    }
    else{
        ss << "\n该文法不是SLR(1)文法\n";
        ss << "其原因为:\n";
        ss << this->reason;
    }
    //输出SLR(1)分析表
    ss << "SLR(1)分析表为\n";
    set<string> sSet;
    for(auto &x: this->forwards){
        for(auto &y: x.second){
            sSet.insert(y.first);
        }
    }
    for(auto &x: this->backs){
        for(auto &y: x.second){
            sSet.insert(y.first);
        }
    }
    ss.setf(ios::left);
    ss << setw(3) << " ";
    for(auto &s: sSet){
        ss << setw(20) << s;
    } 
    ss << '\n';
    for(int i=0; i<this->DFA_nodes.size(); ++i){
        ss << setw(3) << i;
        for(auto &s: sSet){
            if(this->forwards[i].find(s) != this->forwards[i].end()){ //有无移进项
                if(this->ntSet.find(s) != this->ntSet.end()){
                    ss << setw(20) << this->forwards[i][s];
                }
                else{
                    ss << setw(20) << "s" + to_string(this->forwards[i][s]);
                }               
            }
            else if(this->backs[i].find(s) != this->backs[i].end()){ //有无归约项
                Project & p = this->DFA_nodes[i][this->backs[i][s]];
                if(p.key == this->start){
                    ss << setw(20) << "接受";
                }
                else{
                    string a = "r(";
                    a += p.key;
                    a += " -> ";
                    vector<string> & value = this->m[p.key][p.value_num];
                    for(auto &b: value){
                        a += b;
                        a += " ";
                    }  
                    a[a.length()-1] = ')';
                    ss << setw(20) << a;
                }            
            }
            else{
                ss << setw(20) << " ";
            }
        }
        ss << '\n';
    }
    return ss.str();
}

string Grammar::parse(string sentence){
    //定义分析栈s 和 输入队列q
    vector<string> s;
    vector<string> q;
    //初始化用户输入队列
    stringstream ss(sentence + " $");
    string a;
    while(true){
        ss >> a;
        if(a == "$")
            break;
        //划分a中的'(' ')'
        int from = 0;
        int len = 0;
        int i;
        for(i=0; i <a.length(); ++i){
            if(a[i] == '(' || a[i] == ')'){
                len = i - from;
                if(len > 0){
                    q.push_back(a.substr(from, len));
                }
                q.push_back(string(1, a[i]));
                from = i + 1;
            }
        }
        if(from != i){
            q.push_back(a.substr(from));
        }
    }
    q.push_back("$");
    //初始化分析栈
    s.push_back(to_string(0));
    s.push_back("$");
    //分析过程
    ss.clear();
    ss.setf(ios::left);
    ss << setw(10) << "step" << setw(30) << "parse stack" <<
          setw(30) << "input queue" << setw(30) << "action" << '\n';  
    for(int i = 0; ;++i){
        //输出步骤
        ss << setw(10) << i;
        //输出分析栈
        a = "";
        for(int j=s.size()-1; j>=0; --j){
            a += s[j];
            a += " ";
        } 
        ss << setw(30) << a;
        //输出输入队列
        a = "";
        for(auto &x: q){
            a += x;
            a += " ";
        }
            ss << setw(30) << a;
        //状态转换
        int f = stoi(s.front());
        string ts = q.front();
        int t;
        if(this->forwards[f].find(ts) != this->forwards[f].end()){
            q.erase(q.begin());
            t = this->forwards[f][ts];
            s.insert(s.begin(), ts);
            s.insert(s.begin(), to_string(t));
            ss << setw(30) << "移进" + to_string(t) << '\n';
        }
        else if(this->backs[f].find(ts) != this->backs[f].end()){
            t = this->backs[f][ts];
            Project &p = this->DFA_nodes[f][t];
            string &key = p.key;
            if(this->start == key){
                ss << setw(30) << "接受";
                ss << '\n';
                break; 
            }
            vector<string> value = this->m[key][p.value_num];
            if(value[0] != "ε"){
                for(int i=0; i<value.size()*2; ++i){
                    s.erase(s.begin());
                }   
            }
            
            f = stoi(s.front());
            t = this->forwards[f][key];
            s.insert(s.begin(), key);
            s.insert(s.begin(), to_string(t));
            a = "";
            a += "用";
            a += key;
            a += " -> ";
            for(auto &x: value){
                a += x;
                a += " ";
            }
            a += "归约";
            ss << setw(30) << a;
            ss << '\n';
        }
        else{
            ss << setw(30) << "出错";
            break;
        }
    }
    return ss.str();
}

set<string> Grammar::get_first(string key){
    if(this->ntSet.find(key) == this->ntSet.end()){
        return set<string>{key};
    }
    return this->m_first[key];
}

set<string> Grammar::get_follow(string key){
    return m_follow[key];
}

set<string> Grammar::first(vector<string> value){
    set<string> first_set;
    int k;
    for(k=0; k<value.size(); ++k){
        set<string> first_set_k = this->get_first(value[k]);
        for(auto &s: first_set_k){
            first_set.insert(s);
        }
        if(first_set_k.find("ε") == first_set_k.end()){
            break;
        }
        first_set.erase("ε");
    }
    if(k == value.size()){
        first_set.insert("ε");
    }
    return first_set;
}

void Grammar::first_follow_all(){
    //求所有非终结符的first集合
     //初始化
    for(auto &nt: this->ntSet){
        this->m_follow[nt] = set<string>();
    }
    bool isChange = true;
    while(isChange){
        isChange = false;
        for(auto &p: this->m){
            string key = p.first;
            vector<vector<string>> values = p.second;
            for(auto &value: values){
                int k;
                for(k=0; k<value.size(); ++k){
                    auto && first_set_k = this->get_first(value[k]);
                    for(auto &s: first_set_k){
                        if((s!="ε") && (this->m_first[key].find(s) == this->m_first[key].end())){
                            this->m_first[key].insert(s);
                            isChange = true;
                        }
                    }
                    if(first_set_k.find("ε") == first_set_k.end()){
                        break;
                    }
                }
                if(k == value.size() and (this->m_first[key].find("ε") == this->m_first[key].end())){
                    this->m_first[key].insert("ε");
                    isChange = true;
                }
            }
        }
    }
    //求所有非终结符的follow集合
     //初始化
    for(auto &nt: this->ntSet){
        this->m_follow[nt] = set<string>();
    }
    this->m_follow[this->start].insert("$");
    isChange = true;
    while(isChange){
        isChange = false;
        for(auto &p: this->m){ //遍历每一个产生式p
            string key = p.first;
            vector<vector<string>> values = p.second;
            for(auto &value: values){ 
                //遍历value中每一个非终结符
                for(auto it=value.begin(); it!=value.end(); ++it){
                    if(this->ntSet.find(*it) != this->ntSet.end()){
                        //特殊情况
                        if(it+1 == value.end()){
                            for(auto &x: this->get_follow(key)){
                                if(this->m_follow[*it].find(x) == this->m_follow[*it].end()){
                                    this->m_follow[*it].insert(x);   
                                    isChange = true;
                                }   
                            }
                        }
                        else{
                            set<string> first_set = this->first(vector<string>(it+1, value.end()));
                            for(auto &x: first_set){
                                if(this->m_follow[*it].find(x) == this->m_follow[*it].end() and x != "ε"){
                                    this->m_follow[*it].insert(x);   
                                    isChange = true;
                                }  
                            }
                            if(first_set.find("ε") != first_set.end()){ //如果有ε符号
                                this->m_follow[*it].erase("ε");
                                for(auto &x: this->get_follow(key)){
                                    if(this->m_follow[*it].find(x) == this->m_follow[*it].end()){
                                        this->m_follow[*it].insert(x);   
                                        isChange = true;
                                    }  
                                }
                            }  
                        }
                        
                    }
                }
            }
        }
    }
}

void Grammar::node_relationship(){
    reason = "";
    //初始化
    vector<Project> node{Project{this->start, 0, 0, 1}};
    this->DFA_nodes.push_back(node);
    this->isSLR1 = true;
    //循环处理每一个结点i
    for(int i=0; i<this->DFA_nodes.size(); ++i){
        this->forwards[i] = map<string, int>();
        this->backs[i] = map<string, int>();
        //扩展该结点
        extend(i);    
        //遍历该结点的每一个项目
        for(int j=0; j<this->DFA_nodes[i].size(); ++j){
            //处理归约项
            Project &p = this->DFA_nodes[i][j];
            if(p.type == 2){
                set<string> follow_set = this->get_follow(p.key);
                for(auto &s: follow_set){
                    if(this->backs[i].find(s) != this->backs[i].end()){
                        this->isSLR1 = false;
                        stringstream ss;
                        ss << "第" << i << "个结点中归约项目的follow集合有交集\n";    
                        reason += ss.str();  
                    }
                    this->backs[i][s] = j;
                }
            }
            //处理移进项
            else{
                string t = this->m[p.key][p.value_num][p.index];
                Project newP(p.key, p.value_num, p.index+1, 1);
                if(newP.index >= this->m[p.key][p.value_num].size()){
                    newP.type = 2;
                }
                if(this->forwards[i].find(t)==this->forwards[i].end()){//转换t未存在
                    int k=0;
                    for(k=0; k<this->DFA_nodes.size(); ++k){
                        vector<Project> & n = this->DFA_nodes[k];
                        if(find(n.begin(), n.end(), newP) != n.end()){
                            break;
                        }
                    }
                    if(k < this->DFA_nodes.size()){ //项目已经存在
                        this->forwards[i][t] = k;
                    }
                    else{ //项目未存在
                        //新建节点
                        this->DFA_nodes.push_back(vector<Project>{newP});
                        this->forwards[i][t] = this->DFA_nodes.size()-1;
                    }
                }
                else{ //转换t存在
                    int k = this->forwards[i][t];
                    vector<Project> & n = this->DFA_nodes[k];
                    if(find(n.begin(), n.end(), newP) == n.end()){ //项目不存在
                        this->DFA_nodes[k].push_back(newP);
                    }
                }
            }
        }
    }
    if(this->isSLR1){
        for(int i=0; i<this->DFA_nodes.size(); ++i){
            set<string> set1, set2, result;
            for(auto &x: this->forwards[i]){
                set1.insert(x.first);
            }
            for(auto &x: this->backs[i]){
                set2.insert(x.first);
            }
            set_intersection(begin(set1), end(set1), begin(set2), end(set2),
              inserter(result, begin(result)));
            if(result.size() != 0){
                this->isSLR1 = false;
                stringstream ss;
                ss << "第" << i << "个结点的移进项的first集合和归约项的follow集合有交集\n";
                reason += ss.str();
            }
        }
    }
}

void Grammar::extend(int k){
    vector<Project> &node = this->DFA_nodes[k];
    for(int i=0; i<node.size(); ++i){
        Project &p = node[i];
        if(p.type == 1){
            string extend_s = this->m[p.key][p.value_num][p.index];
            if(this->ntSet.find(extend_s) != this->ntSet.end()){
                //可扩展的符号
                vector<vector<string>> &values = this->m[extend_s];
                //遍历extend_s对应的每一个value
                for(int j=0; j<values.size(); ++j){
                    Project addP(extend_s, j, 0, 1);
                    if(values[j][0] == "ε"){//extend_s -> ε特殊情况
                        addP.index = 1;
                        addP.type = 2;
                    }   
                    //判断有无重复
                    if(find(node.begin()+i, node.end(), addP) == node.end()){
                        node.push_back(addP);
                    }
                }
            }            
        }
    }
}