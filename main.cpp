//
// Created by Samarth Mahendra on 2/10/26.
//
// (file path, min, max)
// "2024-01-01/0001.parquet", "aaa", "app"
// "2024-01-01/0002.parquet", "baa", "cat"
// "2024-01-01/0003.parquet", "bob", "dad"
// "2024-01-01/0004.parquet", "eel", "fit"

// - > aa to fit


// "2024-01-01/0005.parquet", "goo", "hop"
// "2024-01-01/0006.parquet", "ink", "lit"
// "2024-01-01/0007.parquet", "pop", "sit"
// "2024-01-01/0008.parquet", "run", "zoo"


// "2024-01-02/0001.parquet", "aaa", "app"
// "2024-01-02/0002.parquet", "baa", "cat"
// "2024-01-03/0001.parquet", "www", "yaa"
// "2024-01-04/0001.parquet", "ybb", "zzz"
//
// Lookup
// Input: <"2024-01-01", "website">, <"2024-01-04", "youth">, <"2024-01-01", "buzz">
// Output:
// <"2024-01-01", "website">: ["2024-01-01/0008.parquet"]
// <"2024-01-04", "youth">: ["2024-01-04/0001.parquet"]

//millions of files

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace std;

static constexpr int MAX_FILES = 4;

class FileSystem {
public:
    string start;
    string end;
    bool is_metadata;

    vector<shared_ptr<FileSystem>> children;
    vector<string> files;

    FileSystem(string s, string e, bool meta = false)
        : start(std::move(s)), end(std::move(e)), is_metadata(meta) {}

    bool overlaps(const string& key) const {
        return start <= key && key <= end;
    }

    vector<string> find(const string& key) {
        vector<string> res;
        if (!overlaps(key)) return res;

        if (!is_metadata) {
            res.insert(res.end(), files.begin(), files.end());
            return res;
        }

        for (auto& c : children) {
            auto sub = c->find(key);
            res.insert(res.end(), sub.begin(), sub.end());
        }
        return res;
    }

    void add_file(const string& s, const string& e, const string& filename) {
        if (!is_metadata) {
            files.push_back(filename);

            // Promote to metadata node
            if (files.size() > MAX_FILES) {
                promote();
            }
            return;
        }

        for (auto& c : children) {
            if (c->start <= s && e <= c->end) {
                c->add_file(s, e, filename);
                return;
            }
        }

        // No suitable child → create one
        auto child = make_shared<FileSystem>(s, e, false);
        child->files.push_back(filename);
        children.push_back(child);
    }

private:
    void promote() {
        is_metadata = true;

        for (const auto& f : files) {
            auto child = make_shared<FileSystem>(start, end, false);
            child->files.push_back(f);
            children.push_back(child);
        }

        files.clear();
    }
};


int main() {
    auto root = make_shared<FileSystem>("aaa", "zzz", true);

    root->add_file("aaa", "app", "2024-01-01/0001.parquet");
    root->add_file("baa", "cat", "2024-01-01/0002.parquet");
    root->add_file("bob", "dad", "2024-01-01/0003.parquet");
    root->add_file("eel", "fit", "2024-01-01/0004.parquet");
    root->add_file("run", "zoo", "2024-01-01/0008.parquet");

    auto res = root->find("website");
    for (auto& f : res) cout << f << "\n";
}