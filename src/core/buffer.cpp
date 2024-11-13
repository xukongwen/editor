#include "buffer.hpp"
#include <fstream>
#include <sstream>

namespace editor {

void Buffer::insert(size_t pos, std::string_view text) {
    if (text.empty()) return;
    
    // 创建编辑操作记录
    EditOperation op{
        pos,
        "",                // 没有被替换的文本
        std::string(text),
        selection_,
        Selection(pos + text.length()) // 新的光标位置
    };
    
    // 执行插入
    rope_.insert(pos, text);
    selection_ = op.newSelection;
    modified_ = true;
    
    // 记录操作用于撤销
    pushEdit(std::move(op));
}

void Buffer::erase(size_t pos, size_t len) {
    if (len == 0) return;
    
    // 保存将被删除的文本
    std::string oldText = rope_.substr(pos, len);
    
    // 创建编辑操作记录
    EditOperation op{
        pos,
        std::move(oldText),
        "",              // 没有新文本
        selection_,
        Selection(pos)   // 新的光标位置
    };
    
    // 执行删除
    rope_.erase(pos, len);
    selection_ = op.newSelection;
    modified_ = true;
    
    // 记录操作用于撤销
    pushEdit(std::move(op));
}

void Buffer::replace(size_t pos, size_t len, std::string_view text) {
    // 保存将被替换的文本
    std::string oldText = rope_.substr(pos, len);
    
    // 创建编辑操作记录
    EditOperation op{
        pos,
        std::move(oldText),
        std::string(text),
        selection_,
        Selection(pos + text.length()) // 新的光标位置
    };
    
    // 执行替换
    rope_.erase(pos, len);
    rope_.insert(pos, text);
    selection_ = op.newSelection;
    modified_ = true;
    
    // 记录操作用于撤销
    pushEdit(std::move(op));
}

void Buffer::setCursor(size_t pos) {
    pos = std::min(pos, length());
    selection_ = Selection(pos);
}

void Buffer::setSelection(size_t start, size_t end) {
    start = std::min(start, length());
    end = std::min(end, length());
    selection_ = Selection(start, end);
}

bool Buffer::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    
    // 读取文件内容
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // 清除当前内容和历史记录
    rope_ = Rope();
    undoStack_.clear();
    redoStack_.clear();
    
    // 插入新内容
    if (!content.empty()) {
        rope_.insert(0, content);
    }
    
    selection_ = Selection(0);
    modified_ = false;
    
    return true;
}

bool Buffer::saveToFile(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;
    
    std::string content = rope_.to_string();
    file.write(content.c_str(), content.length());
    
    return file.good();
}

void Buffer::undo() {
    if (undoStack_.empty()) return;
    
    // 获取最后一个编辑操作
    EditOperation op = std::move(undoStack_.back());
    undoStack_.pop_back();
    
    // 创建重做操作
    EditOperation redoOp{
        op.position,
        std::move(op.newText),
        std::move(op.oldText),
        op.newSelection,
        op.oldSelection
    };
    redoStack_.push_back(std::move(redoOp));
    
    // 恢复之前的状态
    if (!op.newText.empty()) {
        rope_.erase(op.position, op.newText.length());
    }
    if (!op.oldText.empty()) {
        rope_.insert(op.position, op.oldText);
    }
    
    selection_ = op.oldSelection;
    modified_ = !undoStack_.empty();
}

void Buffer::redo() {
    if (redoStack_.empty()) return;
    
    // 获取最后一个重做操作
    EditOperation op = std::move(redoStack_.back());
    redoStack_.pop_back();
    
    // 将操作放回撤销栈
    undoStack_.push_back(std::move(op));
    
    // 重新应用操作
    if (!op.oldText.empty()) {
        rope_.erase(op.position, op.oldText.length());
    }
    if (!op.newText.empty()) {
        rope_.insert(op.position, op.newText);
    }
    
    selection_ = op.newSelection;
    modified_ = true;
}

void Buffer::pushEdit(EditOperation&& op) {
    undoStack_.push_back(std::move(op));
    clearRedoStack();
}

void Buffer::clearRedoStack() {
    redoStack_.clear();
}

} // namespace editor 