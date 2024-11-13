#pragma once

#include "rope.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace editor {

// 表示一个选择范围
struct Selection {
    size_t start;  // 选择起始位置
    size_t end;    // 选择结束位置
    
    Selection(size_t pos = 0) : start(pos), end(pos) {}
    Selection(size_t start_, size_t end_) : start(start_), end(end_) {}
    
    // 获取选择范围的长度
    size_t length() const { return end >= start ? end - start : start - end; }
    
    // 检查是否有选择范围
    bool hasSelection() const { return start != end; }
    
    // 获取选择范围的开始和结束位置（保证 first <= second）
    std::pair<size_t, size_t> range() const {
        return end >= start ? std::make_pair(start, end) : std::make_pair(end, start);
    }
};

// 表示一个编辑操作
struct EditOperation {
    size_t position;           // 操作位置
    std::string oldText;       // 被替换/删除的文本
    std::string newText;       // 新插入的文本
    Selection oldSelection;    // 操作前的选择范围
    Selection newSelection;    // 操作后的选择范围
};

class Buffer {
public:
    Buffer() = default;
    
    // 基本编辑操作
    void insert(size_t pos, std::string_view text);
    void erase(size_t pos, size_t len);
    void replace(size_t pos, size_t len, std::string_view text);
    
    // 光标和选择
    void setCursor(size_t pos);
    void setSelection(size_t start, size_t end);
    const Selection& getSelection() const { return selection_; }
    
    // 内容访问
    char at(size_t pos) const { return rope_.at(pos); }
    std::string substr(size_t pos, size_t len) const { return rope_.substr(pos, len); }
    size_t length() const { return rope_.length(); }
    std::string text() const { return rope_.to_string(); }
    
    // 文件操作
    bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;
    
    // 撤销/重做
    void undo();
    void redo();
    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }
    
    // 修改状态
    bool isModified() const { return modified_; }
    void setModified(bool modified) { modified_ = modified; }

private:
    Rope rope_;                            // 文本内容
    Selection selection_{0};               // 当前选择范围
    std::vector<EditOperation> undoStack_; // 撤销栈
    std::vector<EditOperation> redoStack_; // 重做栈
    bool modified_ = false;                // 修改标志
    
    // 辅助函数
    void pushEdit(EditOperation&& op);
    void clearRedoStack();
};

} // namespace editor 