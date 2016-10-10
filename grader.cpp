#include <iostream>
#include <vector>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "colormod.h"


using namespace std;
#define MSG_MAX 256
#define STAT_ERR -1
#define STAT_RN 1
#define STAT_TL 2
#define STAT_RE 3
#define STAT_WA 4
#define STAT_AC 5

map<string, string> path_map;

class Record{
    private:
        string id;
        bool submitted;
        int nCase;
        vector<int> run_stats;
        vector<int> diff_stats;
    public:
        Record(int _nCase){
            id = "";
            submitted = false;
            nCase = _nCase;
            run_stats.reserve(nCase);
            diff_stats.reserve(nCase);
        }
        Record(const Record& record){
            id = record.id;
            submitted = record.submitted;
            nCase = record.nCase;
            /*run_stats(record.run_stats);
              diff_stats(record.diff_stats);*/
            for(int i = 0; i < record.run_stats.size(); i++)
                run_stats.push_back(record.run_stats[i]);
            for(int i = 0; i < record.diff_stats.size(); i++)
                diff_stats.push_back(record.diff_stats[i]);

        }
        string getID(){ return id; }
        void setID(const string _id){ id = _id;}
        bool isSubmitted(){return submitted;}
        void setSubmitted(bool _submitted){submitted = _submitted;}
        void setRstat(int idx, int stat){
            //if(idx < run_stats.capacity())
            //    run_stats[idx] = stat;
            run_stats.push_back(stat);
        }
        int getRstat(int idx){
            if(idx < run_stats.size())
                return run_stats[idx];
            return 0;
        }
        vector<int> getRstats(){
            return run_stats;
        }
        void setDstat(int idx, int stat){
            //if(idx < diff_stats.capacity())
            //    diff_stats[idx] = stat;
            diff_stats.push_back(stat);
            //cout << "d " << diff_stats.capacity() << " " << diff_stats[idx] << endl;
        }
        int getDstat(int idx){
            if(idx < diff_stats.size())
                return diff_stats[idx];
            return 0;
        }
        vector<int> getDstats(){
            return diff_stats;
        }
};

bool isFileExist(const string file_path){
    ifstream f(file_path.c_str());
    bool ret = f.good();
    f.close();
    return ret;
}

int run(const string file_path, 
        const string input_path, 
        const string result_path, 
        const int limit_time){

    int pid;
    char cmd[256];
    pid = fork();

    //string input_path = "./PA/PA_00/input/input01";
    if(pid > 0){

        /*int returnStatus;    
          waitpid(pid, &returnStatus, 0);
        //cout << "parent process : " << getpid() << ", child: " << pid << " " << returnStatus << endl;
        switch(WEXITSTATUS(returnStatus)){
        case 0: return STAT_RN;
        case 1: return STAT_RE;
        case 130: return STAT_TL;
        }*/
        int timer_pid = fork();
        int r_pid, status;
        int returnStatus;
        if(timer_pid > 0){
            waitpid(pid, &returnStatus, 0);
            //cout << "return status: " << WEXITSTATUS(returnStatus) << " " <<returnStatus << endl;

            r_pid = waitpid(timer_pid, &status, WNOHANG);
            if(r_pid == 0) kill(timer_pid, SIGKILL);
            switch(WEXITSTATUS(returnStatus)){
                case 0: return STAT_RN;
                case 1: return STAT_RE;
                case 130: return STAT_TL;
            }

        }else if(timer_pid ==0){
            sleep(limit_time);
            //cout << "sleep out " << pid << endl;
            status = kill(pid, 0);
            if(status == 0) status = kill(pid, SIGINT);
            exit(status);
        }

    }else if(pid == 0){
        //cout << "child process : " << getpid() << endl;
        //sprintf(cmd, "java -classpath %s Main < %s", file_path.c_str(), input_path.c_str());
        //system(cmd);
        //exit(0);
        int fd = open(input_path.c_str(), O_RDONLY);
        int fd_out = open(result_path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR );
        dup2(fd, 0); // stdin redirection
        dup2(fd_out, 1);
        dup2(fd_out, 2);
        string package_name = path_map["package"];
        execl("/usr/bin/java", "java", "-classpath", file_path.c_str(), package_name.c_str(), 0);
        //exit(0);
    }

    return STAT_ERR;
}

int diff(const string out_file_path, 
        const string ret_file_path,
        const string diff_file_path){

    char cmd[256];
    //sprintf(cmd, "diff %s %s > %s", out_file_path.c_str(), ret_file_path.c_str(), diff_file_path.c_str());
    sprintf(cmd, "bash -c \"diff <(sed 's/[[:blank:]]//g' %s | tr A-Z a-z | tr -d '.!') <(sed 's/[[:blank:]]//g' %s | tr A-Z a-z | tr -d '.!') > %s\"", out_file_path.c_str(), ret_file_path.c_str(), diff_file_path.c_str());
    int stat = system(cmd);
    //cout << "diff stat " << stat << " " << WEXITSTATUS(stat) << endl;
    stat = WEXITSTATUS(stat);

    if(stat == 0) return STAT_AC;
    else if(stat == 1) return STAT_WA;
    return STAT_ERR;
}

string getStatStr(int stat){
   if(stat == STAT_TL) return "TL";
   if(stat == STAT_RE) return "RE";
   if(stat == STAT_WA) return "WA";
   if(stat == STAT_AC) return "AC";
   return ""; 
}

void write_log(const string log_path, vector<Record>& log, int nCase, int point){
    ofstream log_file;
    log_file.open(log_path.c_str());

    log_file<<"id";
    log_file<<",submitted";
    for(int k = 0; k < nCase; k++)
        log_file<<","<<k+1;
    log_file<<",total,stat"<<endl;

    for(int i = 0; i < log.size(); i++){
        //Record record(log[i]);
        //printf("%s", record.getID());
        log_file << log[i].getID();
        log_file << "," << (int) log[i].isSubmitted();
        
        if(log[i].isSubmitted()){
            vector<int> rets = log[i].getDstats();
            int total = 0;
            for(int k = 0; k < nCase; k++){
                int ret = 0;
                if(rets[k] == STAT_AC) ret = 1;
                total += ret*point;
                log_file << "," << ret*point << "(" << getStatStr(rets[k]) <<")";
            }
            log_file << "," << total << endl;
        }else{
            int total = 0;
            for(int k = 0; k < nCase; k++)
                log_file << "," << 0 * point;
            log_file << "," << total << endl;
        }
    }

    log_file.close();

}

string toString(int stat){

    switch(stat){
        case STAT_AC: return "\033[32mACCEPT\033[39m";
        case STAT_RE: return "\033[31mRUNTIME_ERROR\033[39m";
        case STAT_TL: return "\033[31mTIME_LIMIT\033[39m";
        case STAT_RN: return "\033[34mRUN_COMPLETIME\033[39m";
        case STAT_WA: return "\033[31mWRONG_ANSWER\033[39m";
    }
    return "";
}

string toStringAbb(int stat){
    switch(stat){
        case STAT_AC: return "AC";
        case STAT_RE: return "RE";
        case STAT_TL: return "TL";
        case STAT_RN: return "RN";
        case STAT_WA: return "WA";
    }
    return "";

}

vector< pair<string, string> > readStudents(const string path){
    vector< pair<string, string> > list;
    ifstream student_file;
    student_file.open(path.c_str());
    string id, name, line;

    while(getline(student_file, line)){
        istringstream iss(line);
        getline(iss, id, ',');
        getline(iss, name, ',');
        list.push_back( pair<string, string> (id, name));
    }

    student_file.close();
    return list;
}

void pl(const string tag, const string msg){

    Color::Modifier red(Color::FG_RED);
    Color::Modifier def(Color::FG_DEFAULT);

    cout << "[" << red << tag << def << "] " << msg << endl;
}

bool read_config(){
    string config_path = "./config";
    fstream file;
    file.open(config_path.c_str());

    string line, key, value;
    while(getline(file, line)){
        stringstream ss(line);
        ss >> key >> value;
        path_map[key] = value;
    }

    file.close();

    string list_path = path_map["list_path"];

    file.open(list_path.c_str());

    if(file.fail()) return false;
    file.close();

    string package =path_map["package"];
    
    if(package.length() == 0) return false;

    return true;
}

int main(int argc, const char *argv[])
{
    // listing all jar files
    //const char* JARS_PATH = "./PA/PA_00/jars";
    //const char* STUDENT_PATH = "./students.csv";
    /*DIR *dir;
      struct dirent* ent;
      if((dir = opendir(JARS_PATH))!=NULL){
      while((ent = readdir(dir)) != NULL){
      if(ent->d_type != DT_DIR){
      printf("%s\n", ent->d_name);
      }
      }
      closedir(dir);
      }*/

    if(argc < 2){
        cout << "Please input the ID of the programming assignment: 0 ~ 5." << endl;
        return 1;
    }

    int pno = 0;
    sscanf(argv[1], "%d", &pno);

    if(pno < 0 || pno > 6){
        cout << "Please input the ID of the programming assignment: 0 ~ 5." << endl;
        return 1;
    }


    if(!read_config()){
        cout << "Configuration fails! check ./config file." << endl;
        return 1;
    }

    vector< Record > log;
    char msg[256];

    const int limit_time = 20; //2 seconds
    
    
    char str_pa_no[MSG_MAX] = "";
    sprintf(str_pa_no, "PA_%02d", pno);

    string PA_NO(str_pa_no);
    string JARS_PATH = "./PA/"+PA_NO+"/jars";
    string STUDENT_PATH = path_map["list_path"];
    string LOG_PATH = "./PA/"+PA_NO+"/log.csv";
    string INPUT_PATH = "./PA/"+PA_NO+"/input";
    string OUTPUT_PATH = "./PA/"+PA_NO+"/output";
    string RESULT_PATH = "./PA/"+PA_NO+"/result";
    string DIFF_PATH = "./PA/"+PA_NO+"/diff";
    
    
    int nCase = 10;
    int point = 10;

    vector< pair<string, string> > student_list = readStudents(STUDENT_PATH);

    //ifstream student_file;
    //student_file.open(STUDENT_PATH);
    string id, name, line;
    //char del;
    //while(getline(student_file, line)){
    //    istringstream iss(line);
    //    getline(iss, id, ',');
    //getline(iss, name, ',');
    //cout << id << " " << name << eilndl;
    for(int i = 0; i < student_list.size(); i++){
        pair<string, string> student = student_list[i];
        id = student.first;
        name = student.second;

        //cout << "Start grading for " << red << id << def << endl;
        pl(id, "Start grading.");

        Record record(nCase);
        record.setID(id);

        string jar_file_name = PA_NO+"_"+id+".jar";
        string jar_file_path = JARS_PATH+"/"+jar_file_name;

        if(!isFileExist(jar_file_path)){
            //cout << jar_file_name << " does not exist." << endl;
            pl(id, jar_file_path + " does not exist - \033[31mNON-SUBMITTED\033[39m");
            record.setSubmitted(false);
            goto END_GRADING;
        }else{
            record.setSubmitted(true);
            //run.
            //string in_file_path, ret_file_path;
            char in_file_path[256];
            char out_file_path[256];
            char ret_file_path[256];
            char diff_file_path[256];
            for(int i = 0; i < nCase; i++){
                sprintf(in_file_path, "%s/input%02d.txt", INPUT_PATH.c_str(), i);
                sprintf(ret_file_path, "%s/PA_%02d_%s_result%02d", RESULT_PATH.c_str(), pno, id.c_str(), i);
                //cout << in_file_path << endl;
                //cout << ret_file_path << endl;
                int stat = run(jar_file_path, in_file_path, ret_file_path, limit_time);
                sprintf(msg, "%02d-th run result: %s", i, toString(stat).c_str());
                pl(id, msg);
                record.setRstat(i, stat);
            }

            //diff
            for(int i = 0; i < nCase; i++){
                if(record.getRstat(i) == STAT_RN){
                    sprintf(out_file_path, "%s/output%02d.txt", OUTPUT_PATH.c_str(), i);
                    sprintf(ret_file_path, "%s/PA_%02d_%s_result%02d", RESULT_PATH.c_str(), pno, id.c_str(), i);
                    sprintf(diff_file_path, "%s/PA_%02d_%s_diff%02d", DIFF_PATH.c_str(), pno, id.c_str(), i);
                    int stat = diff(out_file_path, ret_file_path, diff_file_path);
                    sprintf(msg, "%02d-th diff result: %s", i, toString(stat).c_str() );
                    pl(id, msg);
                    record.setDstat(i, stat);
                }else{
                    sprintf(msg, "%02d-th diff result: %s", i, toString(record.getRstat(i)).c_str());
                    pl(id, msg);
                    record.setDstat(i, record.getRstat(i));
                }
            }

            //writing record. 
        } //if

END_GRADING:
        log.push_back(record);
        pl(log[i].getID(), "End grading.");
        cout << endl;
    } //for
    //logging
    //header
    write_log(LOG_PATH, log, nCase, point);

    return 0;
}
