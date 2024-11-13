#include "rope.hpp"
#include <algorithm>
#include <stdexcept>

namespace editor {

// Node 构造函数实现
Rope::Node::Node(std::string_view text)
    : data(text)
    , length(text.length())
    , weight(text.length())
    , left(nullptr)
    , right(nullptr) {
}

Rope::Node::Node(std::shared_ptr<Node> l, std::shared_ptr<Node> r)
    : data()
    , length((l ? l->length : 0) + (r ? r->length : 0))
    , weight(l ? l->length : 0)
    , left(std::move(l))
    , right(std::move(r)) {
}

// Rope 构造函数实现
Rope::Rope(std::string_view text) : length_(text.length()) {
    if (!text.empty()) {
        root_ = std::make_shared<Node>(text);
    }
}

void Rope::insert(size_t pos, std::string_view text) {
    if (pos > length_) {
        throw std::out_of_range("Insert position out of range");
    }
    
    if (text.empty()) {
        return;
    }
    
    root_ = insert_at(root_, pos, text);
    length_ += text.length();
}

void Rope::erase(size_t pos, size_t len) {
    if (pos >= length_) {
        throw std::out_of_range("Erase position out of range");
    }
    
    len = std::min(len, length_ - pos);
    if (len == 0) {
        return;
    }
    
    root_ = erase_range(root_, pos, len);
    length_ -= len;
}

char Rope::at(size_t pos) const {
    if (pos >= length_) {
        throw std::out_of_range("Index out of range");
    }
    return index_of(root_, pos);
}

std::string Rope::substr(size_t pos, size_t len) const {
    if (pos >= length_) {
        return "";
    }
    
    len = std::min(len, length_ - pos);
    std::string result;
    result.reserve(len);
    
    for (size_t i = 0; i < len; ++i) {
        result.push_back(at(pos + i));
    }
    
    return result;
}

std::string Rope::to_string() const {
    std::string result;
    result.reserve(length_);
    collect_text(root_, result);
    return result;
}

// 私有辅助函数实现
std::shared_ptr<Rope::Node> Rope::insert_at(
    std::shared_ptr<Node> node,
    size_t pos,
    std::string_view text
) {
    if (!node) {
        return std::make_shared<Node>(text);
    }
    
    if (pos <= node->weight) {
        auto new_left = insert_at(node->left, pos, text);
        return std::make_shared<Node>(new_left, node->right);
    } else {
        auto new_right = insert_at(node->right, pos - node->weight, text);
        return std::make_shared<Node>(node->left, new_right);
    }
}

std::shared_ptr<Rope::Node> Rope::erase_range(
    std::shared_ptr<Node> node,
    size_t pos,
    size_t len
) {
    if (!node || len == 0) {
        return node;
    }
    
    if (pos < node->weight) {
        auto new_left = erase_range(node->left, pos, std::min(len, node->weight - pos));
        size_t remaining = len - std::min(len, node->weight - pos);
        auto new_right = remaining > 0 ? erase_range(node->right, 0, remaining) : node->right;
        
        if (!new_left && !new_right) return nullptr;
        if (!new_left) return new_right;
        if (!new_right) return new_left;
        return std::make_shared<Node>(new_left, new_right);
    } else {
        auto new_right = erase_range(node->right, pos - node->weight, len);
        if (!new_right) return node->left;
        return std::make_shared<Node>(node->left, new_right);
    }
}

void Rope::collect_text(const std::shared_ptr<Node>& node, std::string& result) {
    if (!node) return;
    
    if (node->is_leaf()) {
        result.append(node->data);
    } else {
        collect_text(node->left, result);
        collect_text(node->right, result);
    }
}

char Rope::index_of(const std::shared_ptr<Node>& node, size_t index) {
    if (!node) {
        throw std::out_of_range("Invalid index");
    }
    
    if (node->is_leaf()) {
        return node->data[index];
    }
    
    if (index < node->weight) {
        return index_of(node->left, index);
    } else {
        return index_of(node->right, index - node->weight);
    }
}

} // namespace editor 