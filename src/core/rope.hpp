#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace editor {

class Rope {
public:
    // 构造函数
    Rope() = default;
    explicit Rope(std::string_view text);
    
    // 基本操作
    void insert(size_t pos, std::string_view text);
    void erase(size_t pos, size_t len);
    char at(size_t pos) const;
    std::string substr(size_t pos, size_t len) const;
    std::string to_string() const;
    
    // 属性访问
    size_t length() const { return length_; }
    bool empty() const { return length_ == 0; }
    
    // 迭代器支持（后续可以添加）
    
private:
    // Rope 节点结构
    struct Node {
        static constexpr size_t LEAF_TARGET_LENGTH = 1024;  // 目标叶子节点大小
        
        std::string data;          // 叶子节点的文本数据
        size_t length;             // 节点表示的总长度
        size_t weight;             // 左子树的总长度
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        
        // 构造函数
        explicit Node(std::string_view text);
        Node(std::shared_ptr<Node> l, std::shared_ptr<Node> r);
        
        // 辅助函数
        bool is_leaf() const { return !left && !right; }
    };
    
    // 私有辅助函数
    static std::shared_ptr<Node> insert_at(std::shared_ptr<Node> node, size_t pos, std::string_view text);
    static std::shared_ptr<Node> erase_range(std::shared_ptr<Node> node, size_t pos, size_t len);
    static std::shared_ptr<Node> rebalance(std::shared_ptr<Node> node);
    static void collect_text(const std::shared_ptr<Node>& node, std::string& result);
    static char index_of(const std::shared_ptr<Node>& node, size_t index);
    
    std::shared_ptr<Node> root_;
    size_t length_ = 0;
};

} // namespace editor 