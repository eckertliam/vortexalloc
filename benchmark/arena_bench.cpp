#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "vortexalloc/allocator.hpp"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <list>
#include <string>
#include <vector>
#include <deque>
#include <unordered_set>
#include <random>
#include <algorithm>


constexpr std::size_t N = 1000000;
constexpr std::size_t SMALL_N = 100000;

struct SmallObject {
    std::uint8_t data[16];
    SmallObject() = default;
    SmallObject(int val) { std::fill_n(data, 16, static_cast<std::uint8_t>(val)); }
};

struct MediumObject {
    std::uint8_t data[128];
    MediumObject() = default;
    MediumObject(int val) { std::fill_n(data, 128, static_cast<std::uint8_t>(val)); }
};

struct LargeObject {
    std::uint8_t data[512];
    LargeObject() = default;
    LargeObject(int val) { std::fill_n(data, 512, static_cast<std::uint8_t>(val)); }
};

struct ASTNode {
    enum class Type { Expression, Statement, Declaration } type;
    std::string value;
    std::vector<std::unique_ptr<ASTNode>> children;
    
    ASTNode(Type t, std::string v) : type(t), value(std::move(v)) {}
};

struct TreeNode {
    int value;
    TreeNode* left;
    TreeNode* right;
};

TEST_CASE("Basic Allocation Performance") {
    BENCHMARK("std::vector<int> - sequential allocation") {
        std::vector<int> v;
        v.reserve(N);
        for (std::size_t i = 0; i < N; ++i) {
            v.push_back(static_cast<int>(i));
        }
        return v.size();
    };

    BENCHMARK("ChunkAllocator<int> - sequential allocation") {
        std::vector<int, ChunkAllocator<int>> v;
        v.reserve(N);
        for (std::size_t i = 0; i < N; ++i) {
            v.push_back(static_cast<int>(i));
        }
        return v.size();
    };
}

TEST_CASE("Small Object Allocation Stress") {
    BENCHMARK("std::vector<SmallObject> - small object stress") {
        std::vector<SmallObject> v;
        v.reserve(N);
        for (std::size_t i = 0; i < N; ++i) {
            v.emplace_back(i);
        }
        return v.size();
    };

    BENCHMARK("ChunkAllocator<SmallObject> - small object stress") {
        std::vector<SmallObject, ChunkAllocator<SmallObject>> v;
        v.reserve(N);
        for (std::size_t i = 0; i < N; ++i) {
            v.emplace_back(i);
        }
        return v.size();
    };
}

TEST_CASE("Mixed Size Allocation Patterns") {
    BENCHMARK("std::allocator - mixed size allocation") {
        std::vector<SmallObject> small_objs;
        std::vector<MediumObject> medium_objs;
        std::vector<LargeObject> large_objs;
        
        small_objs.reserve(N / 3);
        medium_objs.reserve(N / 3);
        large_objs.reserve(N / 3);
        
        for (std::size_t i = 0; i < N / 3; ++i) {
            small_objs.emplace_back(i);
            medium_objs.emplace_back(i);
            large_objs.emplace_back(i);
        }
        return small_objs.size() + medium_objs.size() + large_objs.size();
    };

    BENCHMARK("ChunkAllocator - mixed size allocation") {
        std::vector<SmallObject, ChunkAllocator<SmallObject>> small_objs;
        std::vector<MediumObject, ChunkAllocator<MediumObject>> medium_objs;
        std::vector<LargeObject, ChunkAllocator<LargeObject>> large_objs;
        
        small_objs.reserve(N / 3);
        medium_objs.reserve(N / 3);
        large_objs.reserve(N / 3);
        
        for (std::size_t i = 0; i < N / 3; ++i) {
            small_objs.emplace_back(i);
            medium_objs.emplace_back(i);
            large_objs.emplace_back(i);
        }
        return small_objs.size() + medium_objs.size() + large_objs.size();
    };
}

TEST_CASE("Linked Data Structure Performance") {
    BENCHMARK("std::list<int> - linked structure") {
        std::list<int> l;
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            l.push_back(static_cast<int>(i));
        }
        return l.size();
    };

    BENCHMARK("ChunkAllocator<int> - linked structure") {
        std::list<int, ChunkAllocator<int>> l;
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            l.push_back(static_cast<int>(i));
        }
        return l.size();
    };
}

TEST_CASE("String Allocation Performance") {
    BENCHMARK("std::vector<std::string> - string allocation") {
        std::vector<std::string> strings;
        strings.reserve(SMALL_N);
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            strings.emplace_back("string_" + std::to_string(i));
        }
        return strings.size();
    };

    BENCHMARK("ChunkAllocator<std::string> - string allocation") {
        std::vector<std::string, ChunkAllocator<std::string>> strings;
        strings.reserve(SMALL_N);
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            strings.emplace_back("string_" + std::to_string(i));
        }
        return strings.size();
    };
}

TEST_CASE("Memory Locality and Cache Performance") {
    BENCHMARK("std::allocator - cache locality test") {
        std::vector<std::vector<int>> matrix;
        matrix.reserve(1000);
        for (std::size_t i = 0; i < 1000; ++i) {
            matrix.emplace_back(1000);
            for (std::size_t j = 0; j < 1000; ++j) {
                matrix[i][j] = static_cast<int>(i + j);
            }
        }
        return matrix.size();
    };

    BENCHMARK("ChunkAllocator - cache locality test") {
        std::vector<std::vector<int, ChunkAllocator<int>>> matrix;
        matrix.reserve(1000);
        for (std::size_t i = 0; i < 1000; ++i) {
            matrix.emplace_back(1000);
            for (std::size_t j = 0; j < 1000; ++j) {
                matrix[i][j] = static_cast<int>(i + j);
            }
        }
        return matrix.size();
    };
}

TEST_CASE("Arena Reset and Reuse Performance") {
    BENCHMARK("ChunkAllocator - reset and reuse pattern") {
        ChunkAllocator<int> alloc(1024 * 1024); // 1MB chunks
        std::size_t total_allocated = 0;
        
        for (std::size_t round = 0; round < 10; ++round) {
            std::vector<int, ChunkAllocator<int>> v(alloc);
            v.reserve(N / 10);
            for (std::size_t i = 0; i < N / 10; ++i) {
                v.push_back(static_cast<int>(i));
            }
            total_allocated += v.size();
            alloc.reset();
        }
        return total_allocated;
    };
}

TEST_CASE("Fragmentation Resistance") {
    BENCHMARK("std::allocator - fragmentation stress") {
        std::vector<std::unique_ptr<std::vector<int>>> containers;
        containers.reserve(1000);
        
        // Create many small containers
        for (std::size_t i = 0; i < 1000; ++i) {
            auto container = std::make_unique<std::vector<int>>();
            container->reserve(100);
            for (std::size_t j = 0; j < 100; ++j) {
                container->push_back(static_cast<int>(i + j));
            }
            containers.push_back(std::move(container));
        }
        return containers.size();
    };

    BENCHMARK("ChunkAllocator - fragmentation stress") {
        std::vector<std::unique_ptr<std::vector<int, ChunkAllocator<int>>>> containers;
        containers.reserve(1000);
        
        // Create many small containers
        for (std::size_t i = 0; i < 1000; ++i) {
            auto container = std::make_unique<std::vector<int, ChunkAllocator<int>>>();
            container->reserve(100);
            for (std::size_t j = 0; j < 100; ++j) {
                container->push_back(static_cast<int>(i + j));
            }
            containers.push_back(std::move(container));
        }
        return containers.size();
    };
}

TEST_CASE("Compiler-like Workload Simulation") {
    BENCHMARK("std::allocator - AST node allocation") {
        std::vector<std::unique_ptr<ASTNode>> ast_nodes;
        ast_nodes.reserve(SMALL_N);
        
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            auto node = std::make_unique<ASTNode>(
                static_cast<ASTNode::Type>(i % 3),
                "node_" + std::to_string(i)
            );
            
            // Add some children to simulate real AST
            for (std::size_t j = 0; j < 5; ++j) {
                node->children.push_back(std::make_unique<ASTNode>(
                    static_cast<ASTNode::Type>((i + j) % 3),
                    "child_" + std::to_string(j)
                ));
            }
            
            ast_nodes.push_back(std::move(node));
        }
        return ast_nodes.size();
    };

    BENCHMARK("ChunkAllocator - AST node allocation") {
        std::vector<std::unique_ptr<ASTNode>> ast_nodes;
        ast_nodes.reserve(SMALL_N);
        
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            auto node = std::make_unique<ASTNode>(
                static_cast<ASTNode::Type>(i % 3),
                "node_" + std::to_string(i)
            );
            
            // Add some children to simulate real AST
            for (std::size_t j = 0; j < 5; ++j) {
                node->children.push_back(std::make_unique<ASTNode>(
                    static_cast<ASTNode::Type>((i + j) % 3),
                    "child_" + std::to_string(j)
                ));
            }
            
            ast_nodes.push_back(std::move(node));
        }
        return ast_nodes.size();
    };
}

TEST_CASE("Random Access Pattern Performance") {
    BENCHMARK("std::allocator - random access pattern") {
        std::vector<std::vector<int>> vectors;
        vectors.reserve(100);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> size_dist(10, 1000);
        std::uniform_int_distribution<> value_dist(0, 10000);
        
        for (std::size_t i = 0; i < 100; ++i) {
            std::vector<int> v;
            v.reserve(size_dist(gen));
            for (std::size_t j = 0; j < v.capacity(); ++j) {
                v.push_back(value_dist(gen));
            }
            vectors.push_back(std::move(v));
        }
        return vectors.size();
    };

    BENCHMARK("ChunkAllocator - random access pattern") {
        std::vector<std::vector<int, ChunkAllocator<int>>> vectors;
        vectors.reserve(100);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> size_dist(10, 1000);
        std::uniform_int_distribution<> value_dist(0, 10000);
        
        for (std::size_t i = 0; i < 100; ++i) {
            std::vector<int, ChunkAllocator<int>> v;
            v.reserve(size_dist(gen));
            for (std::size_t j = 0; j < v.capacity(); ++j) {
                v.push_back(value_dist(gen));
            }
            vectors.push_back(std::move(v));
        }
        return vectors.size();
    };
}

TEST_CASE("Memory Usage Efficiency") {
    BENCHMARK("std::allocator - memory efficiency test") {
        std::vector<std::deque<int>> deques;
        deques.reserve(500);
        
        for (std::size_t i = 0; i < 500; ++i) {
            std::deque<int> dq;
            for (std::size_t j = 0; j < 1000; ++j) {
                dq.push_back(static_cast<int>(i + j));
                dq.push_front(static_cast<int>(i - j));
            }
            deques.push_back(std::move(dq));
        }
        return deques.size();
    };

    BENCHMARK("ChunkAllocator - memory efficiency test") {
        std::vector<std::deque<int, ChunkAllocator<int>>> deques;
        deques.reserve(500);
        
        for (std::size_t i = 0; i < 500; ++i) {
            std::deque<int, ChunkAllocator<int>> dq;
            for (std::size_t j = 0; j < 1000; ++j) {
                dq.push_back(static_cast<int>(i + j));
                dq.push_front(static_cast<int>(i - j));
            }
            deques.push_back(std::move(dq));
        }
        return deques.size();
    };
}

TEST_CASE("Arena Allocator Strengths - Compiler Workloads") {
    BENCHMARK("std::allocator - symbol table allocation") {
        std::unordered_set<std::string> symbol_table;
        symbol_table.reserve(SMALL_N);
        
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            symbol_table.insert("symbol_" + std::to_string(i) + "_" + std::to_string(i * 2));
        }
        return symbol_table.size();
    };

    BENCHMARK("ChunkAllocator - symbol table allocation") {
        std::unordered_set<std::string, std::hash<std::string>, std::equal_to<std::string>, 
                          ChunkAllocator<std::string>> symbol_table;
        symbol_table.reserve(SMALL_N);
        
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            symbol_table.insert("symbol_" + std::to_string(i) + "_" + std::to_string(i * 2));
        }
        return symbol_table.size();
    };
}

TEST_CASE("Arena Allocator Strengths - Tree Structure Performance") {
    BENCHMARK("std::allocator - binary tree construction") {
        std::vector<std::unique_ptr<struct TreeNode>> nodes;
        nodes.reserve(SMALL_N);
        
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            auto node = std::make_unique<struct TreeNode>();
            node->value = i;
            node->left = nullptr;
            node->right = nullptr;
            nodes.push_back(std::move(node));
        }
        return nodes.size();
    };

    BENCHMARK("ChunkAllocator - binary tree construction") {
        std::vector<std::unique_ptr<struct TreeNode>> nodes;
        nodes.reserve(SMALL_N);
        
        for (std::size_t i = 0; i < SMALL_N; ++i) {
            auto node = std::make_unique<struct TreeNode>();
            node->value = i;
            node->left = nullptr;
            node->right = nullptr;
            nodes.push_back(std::move(node));
        }
        return nodes.size();
    };
}

TEST_CASE("Arena Allocator Strengths - Batch Processing") {
    BENCHMARK("std::allocator - batch processing with reset") {
        std::size_t total_processed = 0;
        
        for (std::size_t batch = 0; batch < 10; ++batch) {
            std::vector<std::string> batch_data;
            batch_data.reserve(SMALL_N / 10);
            
            for (std::size_t i = 0; i < SMALL_N / 10; ++i) {
                batch_data.emplace_back("batch_" + std::to_string(batch) + "_item_" + std::to_string(i));
            }
            total_processed += batch_data.size();
        }
        return total_processed;
    };

    BENCHMARK("ChunkAllocator - batch processing with reset") {
        ChunkAllocator<std::string> alloc(1024 * 1024);
        std::size_t total_processed = 0;
        
        for (std::size_t batch = 0; batch < 10; ++batch) {
            std::vector<std::string, ChunkAllocator<std::string>> batch_data(alloc);
            batch_data.reserve(SMALL_N / 10);
            
            for (std::size_t i = 0; i < SMALL_N / 10; ++i) {
                batch_data.emplace_back("batch_" + std::to_string(batch) + "_item_" + std::to_string(i));
            }
            total_processed += batch_data.size();
            alloc.reset();
        }
        return total_processed;
    };
}

TEST_CASE("Arena Allocator Strengths - Memory Pool Efficiency") {
    BENCHMARK("std::allocator - memory pool simulation") {
        std::vector<std::vector<int>> pools;
        pools.reserve(100);
        
        for (std::size_t i = 0; i < 100; ++i) {
            std::vector<int> pool;
            pool.reserve(1000);
            for (std::size_t j = 0; j < 1000; ++j) {
                pool.push_back(static_cast<int>(i * 1000 + j));
            }
            pools.push_back(std::move(pool));
        }
        return pools.size();
    };

    BENCHMARK("ChunkAllocator - memory pool simulation") {
        std::vector<std::vector<int, ChunkAllocator<int>>> pools;
        pools.reserve(100);
        
        for (std::size_t i = 0; i < 100; ++i) {
            std::vector<int, ChunkAllocator<int>> pool;
            pool.reserve(1000);
            for (std::size_t j = 0; j < 1000; ++j) {
                pool.push_back(static_cast<int>(i * 1000 + j));
            }
            pools.push_back(std::move(pool));
        }
        return pools.size();
    };
}

TEST_CASE("Arena Allocator Strengths - Zero-Copy Operations") {
    BENCHMARK("std::allocator - zero-copy data structures") {
        std::vector<std::list<int>> lists;
        lists.reserve(1000);
        
        for (std::size_t i = 0; i < 1000; ++i) {
            std::list<int> list;
            for (std::size_t j = 0; j < 100; ++j) {
                list.push_back(static_cast<int>(i * 100 + j));
            }
            lists.push_back(std::move(list));
        }
        return lists.size();
    };

    BENCHMARK("ChunkAllocator - zero-copy data structures") {
        std::vector<std::list<int, ChunkAllocator<int>>> lists;
        lists.reserve(1000);
        
        for (std::size_t i = 0; i < 1000; ++i) {
            std::list<int, ChunkAllocator<int>> list;
            for (std::size_t j = 0; j < 100; ++j) {
                list.push_back(static_cast<int>(i * 100 + j));
            }
            lists.push_back(std::move(list));
        }
        return lists.size();
    };
}