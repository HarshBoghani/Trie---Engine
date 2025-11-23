#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

bool isValidChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}


set<string> stopWords;

void loadStopWords() {
    ifstream fin("stop-words.txt");
    string s;
    while (fin >> s) {
        string t;
        for (char c : s) {
            if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
                t.push_back(c);
        }
        if (!t.empty()) stopWords.insert(t);
    }
    fin.close();
}

bool isStopWord(const string &s) {
    return (stopWords.find(s)!=stopWords.end());
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    loadStopWords();

    for (auto &p : fs::directory_iterator("./data")) {
        if (!fs::is_regular_file(p) || p.path().extension() != ".txt") continue;

        ifstream fin(p.path());
        if (!fin.is_open()) {
            cerr << "cannot open: " << p.path() << "\n";
            continue;
        }

        vector<string> v;
        string t;
        char c;

        while (fin.get(c)) {
            if (isValidChar(c)) {
                if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
                t.push_back(c);
            } else {
                if (!t.empty() && !isStopWord(t)) {
                    v.push_back(t);
                    t.clear();
                }else{
                    t.clear();
                }
            }
        }
        if (!t.empty() && !isStopWord(t)) {
            v.push_back(t);
        }else{
            t.clear();
        }
        fin.close();

        ofstream fout(p.path(), ios::trunc);
        if (!fout.is_open()) {
            cerr << "cannot write: " << p.path() << "\n";
            continue;
        }

        for (int i = 0; i < (int)v.size(); i++) {
            fout << v[i];
            if (i + 1 < (int)v.size()) fout << ' ';
        }
        fout.close();

        cout << "Processed: " << p.path().filename() << "\n";
    }
}
