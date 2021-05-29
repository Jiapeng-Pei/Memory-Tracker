#include <QApplication>


#include <test.h>

#include <iostream>
#include <filesystem>

using namespace std;
#include <cctype>


#include <fstream>
#include <utility>
#include "vector"
#include "string"
#include "algorithm"
#include <stdio.h>
#include <unistd.h>
#include "cstdlib"
#include <sys/stat.h>



bool isNum(const string& str);
string getMem(string str);
int getNum(string str);
void update_allprocess();
class processMem{
public:
    int id;
    int VmRSS;
    int Tgid;
    string fname;
    processMem(int id, int VmRSS, int Tgid, string fname){
        this->id = id;
        this->VmRSS = VmRSS;
        this->Tgid = Tgid;
        this->fname = fname;
    }
};

bool sort_processMem (processMem a,processMem b) { return (a.VmRSS>b.VmRSS); }

vector<processMem> allprocess;

void Objecttt::onTimeout(){
    update_allprocess();
    model->removeRows(0,model->rowCount());
    for(int i = 0; i < allprocess.size(); i++){
         this->model->setItem(i, 0, new QStandardItem(QString("%1").arg(allprocess[i].id)));
         this->model->setItem(i, 1, new QStandardItem(QString("%1").arg(allprocess[i].VmRSS)));
         this->model->setItem(i, 2, new QStandardItem(QString("%1").arg(allprocess[i].Tgid)));
         this->model->setItem(i, 3, new QStandardItem(QString("%1").arg(allprocess[i].fname.c_str())));
}

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	
	/* 创建表格视图 */
     QTableView *tableView = new QTableView;
    
    // /* 设置表格视图大小 */
    // tableView->resize(850, 400);

    // /* 创建数据模型 */
     QStandardItemModel* model = new QStandardItemModel();
     Objecttt t(tableView, model);

    return a.exec();
}

bool isNum(const string& str){     // whether a string is number
    for(char i : str){
        if(!(i <= 57 && i >= 48)) return false;
    }
    return true;
}

string getMem(string str){       //  'Vem :  xxx'  -> Vem
    string title = str.substr(0, str.find(':'));
    return title;
}

int getNum(string str){   //  extract 'Vem :   365 kb     '  to 365
    str = str.substr(str.find(':') + 1);
    string num = "";
    bool flag = false;
    for(int i = 0; i < str.length(); i ++) {
        if ((str[i] <= 57 && str[i] >= 48)) {
            num += (str[i]);
            flag = true;
        } else if(flag) break;
    }
    return atoi(num.c_str());
}

string getStr(string str){   //  extract 'Vem :   365 kb     '  to 365
    str = str.substr(str.find(':') + 1);
    string num = "";
    bool flag = false;
    for(int i = 0; i < str.length(); i ++) {
        if ((str[i] <= 122 && str[i] >= 65)) {
            num += (str[i]);
            flag = true;
        } else if(flag) break;
    }
    return num;
}

void update_allprocess(){
    allprocess.clear();
    string proc = "/proc";

    for (auto &entry : filesystem::directory_iterator(proc)) {
        string name = entry.path().filename().string();

        if(access(entry.path().string().c_str(), F_OK) == -1) {
            continue;
        }
        if(!isNum(name)) continue;
        // printf("%s\n", name.c_str());
        // for (auto &entry_t : filesystem::directory_iterator("/proc/" + entry.path().filename().string())){}

        string status = "/proc/" + name + "/status", item;
        ifstream in;
        in.open(status, ios::in);
		int tid = -1; string tname = "";
        processMem p(atoi(name.c_str()), -1, -1, "null");
        while(getline(in,item))
        {
            if (getMem(item) == "VmRSS"){
                p.VmRSS = getNum(item);
                break;
            }
            else if(getMem(item) == "Tgid"){
                p.Tgid = getNum(item);
                tid = p.Tgid;
            }
            else if(getMem(item) == "Name"){
                p.fname = getStr(item);
                tname = p.fname;
            }
        }
        in.close();
        if(p.VmRSS == -1){
            string statm = "/proc/" + name + "/statm", tm;
            ifstream in_tm;
            in_tm.open(statm, ios::in);
            getline(in_tm,tm);
            in_tm.close();
            int a = -1, b = -1;
            for(int i = 0; i < tm.size(); i ++){
                if(a == -1 && tm[i] == ' ') a = i;
                else if(a != -1 && tm[i] == ' '){
                    b = i;
                    break;
                }
            }
            tm = tm.substr(a + 1, b);
            p.VmRSS = atoi(tm.c_str());
        }

        
        allprocess.push_back(p);
        string task = "/proc/" + name + "/task", thread;


        //if(access(task.c_str(), F_OK) == -1) continue;

        for (auto &entry_t : filesystem::directory_iterator(task)){
            thread = entry_t.path().filename().string();
            if(atoi(thread.c_str()) == atoi(name.c_str())) {
                continue;
            }
            string status_t = "/proc/" + thread + "/status", item_t;
            ifstream in_t;
            in_t.open(status_t, ios::in);
            processMem p_t(atoi(thread.c_str()), -1, -1, p.fname);
            while(getline(in_t,item_t)) {
                if (getMem(item_t) == "VmRSS"){
                    p_t.VmRSS = getNum(item_t);
                    break;
                }
                else if(getMem(item_t) == "Tgid"){
                    p_t.Tgid = getNum(item_t);
                }
            }
            in_t.close();
            if(p_t.VmRSS == -1){
                string statm = "/proc/" + thread + "/statm", tm;
                ifstream in_tm;
                in_tm.open(statm, ios::in);
                getline(in_tm,tm);
                in_tm.close();
                int a = -1, b = -1;
                for(int i = 0; i < tm.size(); i ++){
                    if(a == -1 && tm[i] == ' ') a = i;
                    else if(a != -1 && tm[i] == ' '){
                        b = i;
                        break;
                    }
                }
                tm = tm.substr(a + 1, b);
                p_t.VmRSS = atoi(tm.c_str());
        }

            allprocess.push_back(p_t);
        }
    }
    sort(allprocess.begin(), allprocess.end(), sort_processMem);

}

