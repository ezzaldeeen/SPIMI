#include <iostream>
#include <filesystem>
#include <fstream>
#include<sstream>
#include <map>
#include <list>
using namespace std;
using namespace filesystem;

list<string> getFileNames(const string & dir_path) {
    list<string> file_paths;
    for (const directory_entry & dir_entry : directory_iterator(dir_path))
        file_paths.push_back(dir_entry.path().filename());
    return file_paths;
}

int getFileSize(const string & file_path) {
    int size;
    ifstream file;
    file.open(file_path);
    file.seekg(0, ios::end);
    size = int(file.tellg());
    file.close();
    return size;
}

string loadFileContent(const string & file_path) {
    ifstream file;
    const int BUFFER_SIZE = getFileSize(file_path);
    char buffer[BUFFER_SIZE];

    file.open(file_path);
    file.read(buffer, BUFFER_SIZE);
    file.close();
    return buffer;
}

string clearToken(const string & token) {
    string cleared_token;
    for (char ch : token) {
        ch = char(tolower(ch));
        if (int(ch) >= 97 && int(ch) <= 122)
            cleared_token += ch;
    }
    return cleared_token;
}

map<string, int> getTokens(const string & content, const int & doc_id) {
    map<string, int> tokens;
    string token;
    for (char itr : content) {
        if (itr == ' ' || itr == '\n') {
            if (token.length() > 1) {
                token = clearToken(token);
                tokens[token] = doc_id;
            }
            token.clear();
            continue;
        }
        token += char(tolower(itr));
    }
    return tokens;
}

void invert_index(const map<string, int> & token_stream,
                  map<string, list<int>> & dictionary) {
    for (const pair<string, int> term_doc : token_stream)
        dictionary[term_doc.first].push_back(term_doc.second);
}

void write_block_to_disk(map<string, list<int>> & dictionary, int block_num) {
    stringstream ss;
    ss << block_num;
    string out_file;
    ss >> out_file;
    ofstream file("../target/" + out_file);
    for (auto & kv : dictionary) {
        file << kv.first;
        for (auto & i : kv.second)
            file << " " << i;
        file << endl;
    }
}

void build_index(const string & base_path) {
    map<string, list<int>> dictionary;
    list<string> file_names = getFileNames(base_path);
    const int BLOCK_SIZE = 400000;
    int current_size = 0;
    int block_num = 1;
    for (const string & file_name : file_names) {
        string file_path = base_path + file_name;
        current_size += getFileSize(file_path);
        if (current_size < BLOCK_SIZE) {
            string loaded_content = loadFileContent(file_path);
            map<string, int> tokens = getTokens(loaded_content,
                                                stoi(file_name));
            invert_index(tokens, dictionary);
        } else {
            write_block_to_disk(dictionary, block_num);
            current_size = 0;
            block_num++;
            dictionary.clear();
        }
    }
}

int main() {
    build_index("../collection/");
    return 0;
}
