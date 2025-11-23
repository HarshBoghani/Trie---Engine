#include "httplib.h"
#include <bits/stdc++.h>

using namespace std;

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

        node->recommend=recs;

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
        return recs;
    }

    void start_build(){
        build_recs_dfs('.',root);
    }

    vector<pair<int,string>> get_recs(string word) {
        TrieNode* current = root;
        vector<pair<int,string>> recs;
        for (char ch : word) {
            int index = getIndex(ch);
            if (!current->children[index]) {
                return recs;
            }
            current = current->children[index];
        }
        return recs = current->recommend;
    }
};



int main() {
    Trie tr;

    vector<string> tp1 = {
        "harsh","harsh","harshxy","harshxy","harshxy",
        "harshxyz","harshxyz","harshxyz","harshxyz",
        "harshtp","harsyrt","harshytiyu","abcsdsr","seziwuad",
        "react", "reading", "real", "render"
    };

    string s = "file";
    for(int i=0;i<tp1.size();i++){
        tr.insert(tp1[i],s);
    }

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

    svr.listen("0.0.0.0", 8080);
}