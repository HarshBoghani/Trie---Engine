#include "httplib.h"
#include <bits/stdc++.h>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

int getIndex(char x){
    if(x>='a' && x<='z') return x-'a'+10;
    return x-'0';
}

const int SIZE = 36;
const int RECOMMENDATIONS_SIZE = 5;

class TrieNode {
public:
    TrieNode* children[SIZE];
    bool isEndOfWord;
    int cntOfWord;
    set<string> presentIn;
    vector<pair<int,string>> recommend;
    TrieNode() {
        isEndOfWord = false;
        cntOfWord=0;
        for (int i = 0; i < SIZE; ++i) {
            children[i] = nullptr;
        }
    }
};

class Trie {
private:
    TrieNode* root;

public:
    Trie() {
        root = new TrieNode();
    }

    void insert(const string& word, string& file) {
        TrieNode* current = root;
        for (char ch : word) {
            int index = getIndex(ch);
            if (!current->children[index]) {
                current->children[index] = new TrieNode();
            }
            current = current->children[index];
        }
        current->presentIn.insert(file);
        current->cntOfWord++;
        current->isEndOfWord = true;
    }

    set<string> search(string word) {
        TrieNode* current = root;
        set<string> listOfFiles;
        for (char ch : word) {
            if (!(ch >= '0' && ch <= '9') && !(ch >= 'a' && ch <= 'z')) {
                return listOfFiles;
            }
            int index = getIndex(ch);
            if (!current->children[index]) {
                return listOfFiles;
            }
            current = current->children[index];
        }
        return listOfFiles = current->presentIn;
    }

    vector<pair<int,string>> build_recs_dfs(char lastLetter, TrieNode* node){

        vector<pair<int,string>> recs;
        set<pair<int,string>> children_recs;

        for(int i=0;i<SIZE;i++){
            if(!(node->children[i])) continue;
            char letter;
            if(i<10) letter = i+'0';
            else letter = i-10+'a';
            vector<pair<int,string>> child_recs = build_recs_dfs(letter,node->children[i]);

            for(int j=0;j<child_recs.size();j++){

                if(children_recs.size() < RECOMMENDATIONS_SIZE) {
                    children_recs.insert({child_recs[j].first,child_recs[j].second});
                }else{
                    if(children_recs.begin()->first < child_recs[j].first){
                        children_recs.erase(children_recs.begin());
                        children_recs.insert({child_recs[j].first,child_recs[j].second});
                    }
                }
            }
        }

        for(auto &it : children_recs){
            string temp = "";
            temp+=lastLetter;
            temp+=it.second;
            recs.push_back({it.first,temp});
        }

        if(node->isEndOfWord){
            string temp = "";
            temp+=lastLetter;
            int currCnt = node->cntOfWord;
            if(children_recs.size() < RECOMMENDATIONS_SIZE) {
                recs.push_back({currCnt,temp});
            }else{
                if(currCnt > children_recs.begin()->first){
                    for(int i=0;i<recs.size();i++){
                        if(recs[i].first==children_recs.begin()->first && recs[i].second==children_recs.begin()->second){
                            recs[i].first=currCnt;
                            recs[i].second=temp;
                        }
                    }
                }
            }
        }

        node->recommend=recs;

        
        return recs;
    }

    void start_build(){
        build_recs_dfs('.',root);
    }

    vector<pair<int,string>> get_recs(string word) {
        TrieNode* current = root;
        vector<pair<int,string>> recs;
        for (char ch : word) {
            if (!(ch >= '0' && ch <= '9') && !(ch >= 'a' && ch <= 'z')) {
                return recs;
            }
            int index = getIndex(ch);
            cout << index << endl;
            if (!current->children[index]) {
                return recs;
            }
            current = current->children[index];
        }
        return recs = current->recommend;
    }

    set<string> OR_search(vector<string> words){
        set<string> files;
        for(auto &word : words){
            set<string> files1 = search(word);
            for(auto &file : files1){
                files.insert(file);
            }
        }
        return files;
    } 

};



int main() {
    Trie tr;

    cout << "Loading files from data directory..." << endl;
    int fileCount = 0;
    for (auto &p : fs::directory_iterator("./data")) {
        if (!fs::is_regular_file(p) || p.path().extension() != ".txt") continue;

        string filename = p.path().filename().string();
        ifstream fin(p.path());
        if (!fin.is_open()) {
            cerr << "Warning: Could not open file: " << p.path() << endl;
            continue;
        }

        string word;
        while (fin >> word) {
            tr.insert(word, filename);
        }
        fin.close();
        fileCount++;
    }
    cout << "Loaded " << fileCount << " files." << endl;

    cout << "Building Trie recommendations..." << endl;
    tr.start_build();
    httplib::Server svr;
    cout << "Server running on port 8080..." << endl;

    svr.Get("/recommend", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        string prefix = "";
        if (req.has_param("q")) {
            prefix = req.get_param_value("q");
        }

        if(prefix.empty()) {
            res.set_content("[]", "application/json");
            return;
        }

        vector<pair<int,string>> recs = tr.get_recs(prefix);

        string json = "[";
        for(size_t i=0; i<recs.size(); ++i){
            string suffix = recs[i].second;
            if(suffix.length() > 0) suffix = suffix.substr(1);

            string fullWord = prefix + suffix;

            json += "{\"word\":\"" + fullWord + "\", \"score\":" + to_string(recs[i].first) + "}";

            if(i < recs.size()-1) json += ",";
        }
        json += "]";

        res.set_content(json, "application/json");
    });

    svr.Get("/search", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        string query = "";
        if (req.has_param("q")) {
            query = req.get_param_value("q");
        }

        if(query.empty()) {
            res.set_content("[]", "application/json");
            return;
        }

        vector<string> words;
        stringstream ss(query);
        string word;
        while (ss >> word) {
            if(!word.empty()) {
                words.push_back(word);
            }
        }

        set<string> files;
        if(words.empty()) {
            res.set_content("[]", "application/json");
            return;
        } else if(words.size() == 1) {
            files = tr.search(words[0]);
        } else {
            files = tr.OR_search(words);
        }

        string json = "[";
        bool first = true;
        for(const string& file : files) {
            if(!first) json += ",";
            json += "\"" + file + "\"";
            first = false;
        }
        json += "]";

        res.set_content(json, "application/json");
    });

    svr.Get("/file", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        string filename = "";
        if (req.has_param("name")) {
            filename = req.get_param_value("name");
        }

        if(filename.empty()) {
            res.set_content("", "text/plain");
            return;
        }

        string filepath = "./data/" + filename;
        ifstream file(filepath);
        if (!file.is_open()) {
            res.set_content("", "text/plain");
            return;
        }

        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();

        res.set_content(content, "text/plain");
    });

    svr.listen("0.0.0.0", 8080);
}