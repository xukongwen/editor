#pragma once

#include "core/buffer.hpp"
#include "core/renderer.hpp"
#include "platform/macos/window.hpp"
#include <memory>
#include <string>

namespace editor {

class Editor {
public:
    Editor();
    ~Editor();

    // 初始化编辑器
    bool init();
    
    // 更新和渲染
    void update();
    void render();
    
    // 文件操作
    bool loadFile(const std::string& path);
    bool saveFile(const std::string& path);
    
    // 编辑操作
    void insertText(std::string_view text);
    void deleteSelection();
    void setSelection(size_t start, size_t end);
    void setCursor(size_t pos);
    
    // 清理资源
    void cleanup();

private:
    // 创建窗口和渲染器
    bool createWindow();
    bool createRenderer();
    
    // 处理输入
    void handleInput();
    
    // 计算文本布局
    void updateLayout();

private:
    std::unique_ptr<Window> window_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Buffer> buffer_;
    
    // 编辑器状态
    struct {
        float scrollX = 0;
        float scrollY = 0;
        float viewportWidth = 0;
        float viewportHeight = 0;
        float lineHeight = 20.0f;
        float charWidth = 8.0f;
        TextStyle textStyle;
    } state_;
    
    bool initialized_ = false;
};

} // namespace editor 