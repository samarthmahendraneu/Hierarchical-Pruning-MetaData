# File Range Lookup using Hierarchical Metadata Index

This project implements a **metadata-driven range lookup system** for efficiently locating files whose key ranges overlap a given search key.

The design mirrors how modern **lakehouse storage systems** (e.g., Apache Hudi / Iceberg / Delta Lake) prune files using hierarchical metadata instead of scanning millions of files.

---

## Problem Statement

Each data file is associated with a **range of values**:

```
(file_path, min_key, max_key)
```

Example:
```
"2024-01-01/0001.parquet", "aaa", "app"
"2024-01-01/0008.parquet", "run", "zoo"
```

Given lookup queries of the form:
```
(date, search_key)
```

Return all files whose range satisfies:
```
min_key <= search_key <= max_key
```

---

## Challenges

- Scale to **millions of files**
- Avoid linear scans
- Support fast pruning using metadata
- Enable recursive filtering

---

## Approach

The system uses a **hierarchical metadata index**:

- **Leaf nodes** store actual file names
- **Metadata nodes** group children by key range
- Nodes are **promoted** to metadata when they exceed a size threshold
- Queries recursively traverse only overlapping ranges

This results in **logarithmic pruning** instead of full scans.

---

## Core Concepts

- Interval overlap checking
- Metadata fan-out control
- Recursive range filtering
- Separation of metadata vs file nodes

---


---

## Why This Design Works

- Avoids scanning millions of files
- Mimics real lakehouse metadata pruning
- Easily extendable to:
  - Interval trees
  - B-trees
  - Bloom filter acceleration
  - Partition-aware pruning



## Time Complexity

| Operation | Complexity |
|----------|------------|
| Insert   | O(log N) average |
| Lookup   | O(log N + K) |

*K = number of matching files*

---

## Future Improvements

- Bloom filters per node
- Parallel traversal


## Build & Run

```bash
g++ -std=c++17 main.cpp -o lookup
./lookup
```

---

## Author

Samarth Mahendra
