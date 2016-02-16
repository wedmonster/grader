#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include "colormod.h"
using namespace std;
Color::Modifier red(Color::FG_RED);
Color::Modifier green(Color::FG_GREEN);
Color::Modifier def(Color::FG_DEFAULT);

vector<string> search(vector<string>& list, string pat){
    vector<string> ret;
    for(int i = 0; i < list.size(); i++){
        string id = list[i];
        int cnt = 0;
        for (int k = 5; k < id.size(); k++) {
            if(pat.c_str()[k-5] == id.c_str()[k]){
                cnt++;
            }
        }
        if(cnt == 5) ret.push_back(id);
    }
    return ret;
}
string searchID(vector<string>& list, string pat){
    vector<string> ret = search(list, pat);

    if(ret.size() == 0){
        cout << "there is no id matched with " << red << pat << def << endl;
        return "";
    }else{
        if(ret.size() == 1)
            cout << "there is id matched with " << green << pat << ": " << ret[0] << def << endl;
    }

    string in_id = ret[0];
    if(ret.size() > 1){
        cout << red << "there are multiple ids, select : " << def << endl;
        for (int i = 0; i < ret.size(); i++)
            cout<<i<<" "<<ret[i]<<endl;
        int idx;
        cin >> idx;
        getchar();
        in_id = ret[idx];
    }

    return in_id;
}
int main(int argc, const char *argv[])
{
    ifstream file;
    vector<string> list;
    string path = "./students.csv";
    file.open(path.c_str());
    string line, id, name;
    while(getline(file, line)){
        istringstream iss(line);
        getline(iss, id, ',');
        getline(iss, name, ',');
        list.push_back(id);
    }
    file.close();

    string save_file_name;
    cout << "The system is ready. Please give a name of the file: ";
    cin >> save_file_name;

    /*int no = 0; 
      for(int i = 0; i < list.size(); i++){
      string src_id = list[i];
      for(int j = i+1; j < list.size(); j++){
      string tar_id = list[j];
      int cnt = 0;       
      for(int k = 5; k < src_id.size(); k++){
      if(src_id.c_str()[k] == tar_id.c_str()[k]){
    //cout<< "o";
    cnt++;
    }else{
    //cout<< "x";
    }
    }
    if(cnt >= 5){
    cout << no++ <<" " <<src_id << " " << tar_id << endl;
    }
    //cout << endl;
    }
    }*/
    string cmd, pat, score;
    map<string, string> log;
    while(getline(cin, line)){
        istringstream iss(line);
        iss >> cmd;
        if(cmd == "0"){
            score = "";
            pat = "";
            iss >> pat >> score;

            string in_id = searchID(list, pat);
            if(in_id == "")  continue;

            if(score == ""){
                cout << red << "no score is entered!" << def << endl;
                continue;
            }

            //id is selected. 
            map<string, string>::iterator it;
            it = log.find(in_id);
            if(it == log.end()){
                log[in_id] = score;
                cout << red << "size: " << log.size() << ", " << green << in_id + "'s score is " + log[in_id] << def << endl;
            }else{
                cout<<"there is an entry: "<<in_id<<", "<<it->second<<endl;
            }
        }else if(cmd == "m"){
            score = "";
            iss >> pat >> score;

            string in_id = searchID(list, pat);
            if(in_id == "")  continue;

            if(score == ""){
                cout << red << "no score is entered!" << def << endl;
                continue;
            }
            map<string, string>::iterator it;
            it = log.find(in_id);
            if(it == log.end()){
                cout << red << "there is no score on log with " << in_id << def << endl;
                cout << "do you want to insert the score with that id?";
                string req = "";
                cin >> req; getchar();
                if(req == "y") {
                    log[in_id] = score;
                    cout << red << "size: " << log.size() << ", " << green << in_id + "'s score is " + log[in_id] << def << endl;
                    
                }
            }else{
                log[in_id] = score;
                cout << red << "size: " << log.size() << ", " << green << in_id + "'s score is modified to " + log[in_id] << def << endl;
            }
        }else if(cmd == "s"){
            ofstream save_file;
            save_file.open(string("./log/" + save_file_name + ".csv").c_str());
            
            save_file << "id," + save_file_name <<endl;
            map<string, string>::iterator it;
            for (int i = 0; i < list.size(); i++) {
                string id = list[i];
                it = log.find(id);
                if(it != log.end()) {
                    //cout<<id<< " : " <<it->second<<endl;
                    save_file << id << "," << it->second << endl;
                }else{
                    save_file << id << "," << 0 << endl;
                }
            }
            
            save_file.close();
            cout << green << "save complete!!" << def << endl;
        }else if(cmd == "l"){
            cout << "Current log size is " << log.size() << endl;
            map<string, string>::iterator it;
            for(it = log.begin(); it != log.end(); it++){
                string id = it->first;
                string score = it->second;
                cout << red << id << " => " << green << score << def <<endl;
            }
        }else if(cmd == "q"){
            return 0;
        }else{
            cout << "cmds are i, s, l, m, q." << endl;
        }
    }

    return 0;
}
