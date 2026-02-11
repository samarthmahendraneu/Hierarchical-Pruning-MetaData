#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>

using namespace std;

static constexpr int MAX_FILES = 3;

struct FileEntry {
    string min_key;
    string max_key;
    string filepath;

    FileEntry(string min_k, string max_k, string path)
        : min_key(move(min_k)), max_key(move(max_k)), filepath(move(path)) {}
};

class FileSystem {
public:
    string start;
    string end;
    bool is_metadata;

    vector<shared_ptr<FileSystem>> children;
    vector<FileEntry> files;

    FileSystem(bool meta = false) : is_metadata(meta) {}

    bool overlaps(const string& key) const {
        if (start.empty()) return true;
        return start <= key && key <= end;
    }

    void update_bounds() {
        if (!is_metadata && !files.empty()) {
            start = files.front().min_key;
            end = files.back().max_key;
        } else if (is_metadata && !children.empty()) {
            start = children.front()->start;
            end = children.back()->end;
        }
    }

    vector<string> find(const string& key) {
        vector<string> res;
        if (!overlaps(key)) return res;

        if (!is_metadata) {
            for (auto& f : files) {
                if (f.min_key <= key && key <= f.max_key) {
                    res.push_back(f.filepath);
                }
            }
        } else {
            for (auto& c : children) {
                if (c->overlaps(key)) {
                    auto sub = c->find(key);
                    res.insert(res.end(), sub.begin(), sub.end());
                }
            }
        }

        return res;
    }

    shared_ptr<FileSystem> add_file(const string& s, const string& e, const string& filename) {
        if (!is_metadata) {
            files.emplace_back(s, e, filename);

            sort(files.begin(), files.end(),
                [](const FileEntry& a, const FileEntry& b) {
                    return a.min_key < b.min_key;
                });

            update_bounds();

            if (files.size() > MAX_FILES) {
                return split();
            }
            return nullptr;
        }

        for (size_t i = 0; i < children.size(); i++) {
            if (children[i]->overlaps(s)) {
                auto new_child = children[i]->add_file(s, e, filename);

                if (new_child) {
                    children.insert(children.begin() + i + 1, new_child);
                    update_bounds();

                    if (children.size() > MAX_FILES) {
                        return split();
                    }
                }

                update_bounds();
                return nullptr;
            }
        }

        auto child = make_shared<FileSystem>(false);
        child->files.emplace_back(s, e, filename);
        child->update_bounds();
        children.push_back(child);

        sort(children.begin(), children.end(),
            [](const auto& a, const auto& b) {
                return a->start < b->start;
            });

        update_bounds();
        return nullptr;
    }

    void print(int depth = 0) const {
        string indent(depth * 2, ' ');
        cout << indent << (is_metadata ? "INTERNAL" : "LEAF")
             << " [" << start << " -> " << end << "]\n";

        if (!is_metadata) {
            for (const auto& f : files) {
                cout << indent << "  " << f.filepath
                     << " [" << f.min_key << " -> " << f.max_key << "]\n";
            }
        } else {
            for (const auto& c : children) {
                c->print(depth + 1);
            }
        }
    }

private:
    shared_ptr<FileSystem> split() {
        size_t mid = is_metadata ? children.size() / 2 : files.size() / 2;

        auto new_node = make_shared<FileSystem>(is_metadata);

        if (is_metadata) {
            new_node->children.assign(children.begin() + mid, children.end());
            children.erase(children.begin() + mid, children.end());
        } else {
            is_metadata = true;

            auto left = make_shared<FileSystem>(false);
            left->files.assign(files.begin(), files.begin() + mid);
            left->update_bounds();

            auto right = make_shared<FileSystem>(false);
            right->files.assign(files.begin() + mid, files.end());
            right->update_bounds();

            children.push_back(left);
            children.push_back(right);
            files.clear();

            update_bounds();
            return nullptr;
        }

        update_bounds();
        new_node->update_bounds();

        return new_node;
    }
};

class PartitionedIndex {
    map<string, shared_ptr<FileSystem>> partitions;

    string extract_partition(const string& filepath) {
        return filepath.substr(0, filepath.find('/'));
    }

public:
    void add_file(const string& filepath, const string& min_k, const string& max_k) {
        string partition = extract_partition(filepath);

        if (partitions.find(partition) == partitions.end()) {
            partitions[partition] = make_shared<FileSystem>(false);
        }

        auto root = partitions[partition];
        auto new_node = root->add_file(min_k, max_k, filepath);

        if (new_node) {
            auto new_root = make_shared<FileSystem>(true);
            new_root->children.push_back(root);
            new_root->children.push_back(new_node);
            new_root->update_bounds();
            partitions[partition] = new_root;
        }
    }

    vector<string> find(const string& partition, const string& key) {
        auto it = partitions.find(partition);
        if (it == partitions.end()) return {};
        return it->second->find(key);
    }

    void print(const string& partition) {
        auto it = partitions.find(partition);
        if (it != partitions.end()) {
            cout << "=== Partition: " << partition << " ===\n";
            it->second->print();
            cout << "\n";
        }
    }
};

int main() {
    PartitionedIndex index;

    cout << "Adding files...\n\n";

    index.add_file("2024-01-01/0001.parquet", "aaa", "app");
    index.add_file("2024-01-01/0002.parquet", "baa", "cat");
    index.add_file("2024-01-01/0003.parquet", "bob", "dad");
    index.add_file("2024-01-01/0004.parquet", "eel", "fit");
    index.add_file("2024-01-01/0005.parquet", "goo", "hop");
    index.add_file("2024-01-01/0006.parquet", "ink", "lit");
    index.add_file("2024-01-01/0007.parquet", "pop", "sit");
    index.add_file("2024-01-01/0008.parquet", "run", "zoo");
    index.add_file("2024-01-02/0001.parquet", "aaa", "app");
    index.add_file("2024-01-02/0002.parquet", "baa", "cat");
    index.add_file("2024-01-03/0001.parquet", "www", "yaa");
    index.add_file("2024-01-04/0001.parquet", "ybb", "zzz");

    index.print("2024-01-01");

    cout << "\n=== Lookups ===\n";

    auto res = index.find("2024-01-01", "website");
    cout << "find('2024-01-01', 'website'): ";
    for (auto& f : res) cout << f << " ";
    cout << "\n";

    res = index.find("2024-01-04", "youth");
    cout << "find('2024-01-04', 'youth'): ";
    for (auto& f : res) cout << f << " ";
    cout << "\n";

    res = index.find("2024-01-01", "buzz");
    cout << "find('2024-01-01', 'buzz'): ";
    for (auto& f : res) cout << f << " ";
    cout << "\n";

    return 0;
}