#import <Cocoa/Cocoa.h>
#include "editor.hpp"
#include <string>

namespace editor {

// 创建一个自定义视图来处理输入
@interface EditorView : NSView {
    editor::Editor* editor_;
}
- (id)initWithEditor:(editor::Editor*)editor frame:(NSRect)frame;
@end

@implementation EditorView

- (id)initWithEditor:(editor::Editor*)editor frame:(NSRect)frame {
    if ((self = [super initWithFrame:frame])) {
        editor_ = editor;
        self.wantsLayer = YES;
    }
    return self;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)event {
    NSString* chars = [event characters];
    NSString* charsIgnoringModifiers = [event charactersIgnoringModifiers];
    NSUInteger modifiers = [event modifierFlags];
    
    // 处理特殊键
    if (modifiers & NSEventModifierFlagCommand) {
        unichar key = [charsIgnoringModifiers characterAtIndex:0];
        switch (key) {
            case 'v': // 粘贴
                {
                    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
                    NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
                    if (string) {
                        editor_->insertText(string.UTF8String);
                    }
                }
                return;
                
            case 'c': // 复制
                // TODO: 实现复制功能
                return;
                
            case 'x': // 剪切
                // TODO: 实现剪切功能
                return;
                
            case 'z': // 撤销
                // TODO: 实现撤销功能
                return;
        }
    }
    
    // 处理回车键
    if ([charsIgnoringModifiers characterAtIndex:0] == NSCarriageReturnCharacter) {
        editor_->insertText("\n");
        return;
    }
    
    // 处理删除键
    if ([event keyCode] == 51) {  // Backspace
        editor_->deleteSelection();
        return;
    }
    
    // 处理普通文本输入
    if (chars.length > 0) {
        editor_->insertText(chars.UTF8String);
    }
}

- (void)mouseDown:(NSEvent *)event {
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    // TODO: 转换坐标到文本位置并设置光标
}

- (void)mouseDragged:(NSEvent *)event {
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    // TODO: 更新选择范围
}

- (void)mouseUp:(NSEvent *)event {
    // TODO: 完成选择
}

@end

Editor::Editor() 
    : window_(nullptr)
    , renderer_(nullptr)
    , buffer_(nullptr)
    , initialized_(false) {
}

Editor::~Editor() {
    cleanup();
}

bool Editor::init() {
    if (initialized_) return true;
    
    // 创建窗口
    if (!createWindow()) {
        return false;
    }
    
    // 创建渲染器
    if (!createRenderer()) {
        return false;
    }
    
    // 创建文本缓冲区
    buffer_ = std::make_unique<Buffer>();
    
    // 设置默认文本样式
    state_.textStyle.fontSize = 14.0f;
    state_.textStyle.lineHeight = 1.2f;
    state_.textStyle.color = 0xFF000000;
    
    initialized_ = true;
    return true;
}

bool Editor::createWindow() {
    window_ = std::make_unique<Window>();
    if (!window_->create("Minimal Text Editor", 1024, 768)) {
        return false;
    }
    
    // 创建并设置编辑器视图
    NSWindow* nsWindow = (__bridge NSWindow*)window_->getNativeWindow();
    NSRect contentRect = [[nsWindow contentView] bounds];
    EditorView* editorView = [[EditorView alloc] initWithEditor:this frame:contentRect];
    [nsWindow setContentView:editorView];
    [nsWindow makeFirstResponder:editorView];
    
    // 设置窗口大小变化回调
    window_->setResizeCallback([this](int width, int height) {
        state_.viewportWidth = width;
        state_.viewportHeight = height;
        updateLayout();
    });
    
    return true;
}

bool Editor::createRenderer() {
    renderer_ = std::make_unique<Renderer>();
    if (!renderer_->init(window_->getRenderView()->metalLayer)) {
        return false;
    }
    return true;
}

void Editor::update() {
    if (!initialized_) return;
    
    handleInput();
    updateLayout();
}

void Editor::render() {
    if (!initialized_) return;
    
    renderer_->beginFrame(state_.viewportWidth, state_.viewportHeight);
    
    // 计算可见行范围
    size_t startLine = state_.scrollY / state_.lineHeight;
    size_t endLine = std::min(
        lines_.size(),
        static_cast<size_t>((state_.scrollY + state_.viewportHeight) / state_.lineHeight + 1)
    );
    
    // 渲染可见的文本行
    std::string text = buffer_->text();
    for (size_t i = startLine; i < endLine; ++i) {
        const LineInfo& line = lines_[i];
        float y = i * state_.lineHeight - state_.scrollY;
        
        if (line.length > 0) {
            renderer_->drawText(
                -state_.scrollX,
                y,
                std::string_view(text.data() + line.start, line.length),
                state_.textStyle
            );
        }
    }
    
    // 渲染光标
    const Selection& sel = buffer_->getSelection();
    if (!sel.hasSelection()) {
        size_t pos = sel.start;
        float cursorX = (pos - start) * state_.charWidth - state_.scrollX;
        float cursorY = (pos / 80) * state_.lineHeight - state_.scrollY;  // 假设每行80字符
        renderer_->drawCursor(cursorX, cursorY, state_.lineHeight);
    }
    
    // 渲染选择区域
    else {
        auto [selStart, selEnd] = sel.range();
        float selX = (selStart % 80) * state_.charWidth - state_.scrollX;
        float selY = (selStart / 80) * state_.lineHeight - state_.scrollY;
        float selWidth = (selEnd - selStart) * state_.charWidth;
        renderer_->drawSelection(selX, selY, selWidth, state_.lineHeight);
    }
    
    renderer_->endFrame();
}

void Editor::handleInput() {
    // TODO: 处理键盘和鼠标输入
}

void Editor::updateLayout() {
    if (!initialized_ || !buffer_) return;
    
    // 清除旧的行信息
    lines_.clear();
    
    std::string text = buffer_->text();
    size_t start = 0;
    size_t lineNumber = 0;
    float maxWidth = 0;
    
    // 计算每一行的信息
    for (size_t i = 0; i <= text.length(); ++i) {
        if (i == text.length() || text[i] == '\n') {
            size_t length = i - start;
            
            // 创建行信息
            LineInfo line;
            line.start = start;
            line.length = length;
            line.lineNumber = lineNumber;
            line.height = state_.lineHeight;
            
            // 计算行宽度
            if (length > 0) {
                // 使用 Core Text 计算实际文本宽度
                CFStringRef string = CFStringCreateWithBytes(
                    kCFAllocatorDefault,
                    reinterpret_cast<const UInt8*>(text.data() + start),
                    length,
                    kCFStringEncodingUTF8,
                    false
                );
                
                CTFontRef font = CTFontCreateWithName(
                    CFSTR("Menlo"),
                    state_.textStyle.fontSize,
                    nullptr
                );
                
                // 创建文本属性
                CFStringRef keys[] = { kCTFontAttributeName };
                CFTypeRef values[] = { font };
                CFDictionaryRef attributes = CFDictionaryCreate(
                    kCFAllocatorDefault,
                    (const void**)&keys,
                    (const void**)&values,
                    1,
                    &kCFTypeDictionaryKeyCallBacks,
                    &kCFTypeDictionaryValueCallBacks
                );
                
                CFAttributedStringRef attrString = CFAttributedStringCreate(
                    kCFAllocatorDefault,
                    string,
                    attributes
                );
                
                CTLineRef lineRef = CTLineCreateWithAttributedString(attrString);
                CGFloat ascent, descent, leading;
                line.width = CTLineGetTypographicBounds(lineRef, &ascent, &descent, &leading);
                
                // 更新最大宽度
                maxWidth = std::max(maxWidth, line.width);
                
                // 清理资源
                CFRelease(lineRef);
                CFRelease(attrString);
                CFRelease(attributes);
                CFRelease(font);
                CFRelease(string);
            } else {
                line.width = 0;
            }
            
            lines_.push_back(line);
            
            start = i + 1;
            lineNumber++;
        }
    }
    
    // 更新内容区域的总大小
    float totalHeight = lines_.size() * state_.lineHeight;
    
    // 调整滚动范围
    if (state_.scrollY < 0) {
        state_.scrollY = 0;
    }
    if (state_.scrollY > totalHeight - state_.viewportHeight) {
        state_.scrollY = std::max(0.0f, totalHeight - state_.viewportHeight);
    }
    
    if (state_.scrollX < 0) {
        state_.scrollX = 0;
    }
    if (state_.scrollX > maxWidth - state_.viewportWidth) {
        state_.scrollX = std::max(0.0f, maxWidth - state_.viewportWidth);
    }
}

bool Editor::loadFile(const std::string& path) {
    if (!initialized_ || !buffer_) return false;
    return buffer_->loadFromFile(path);
}

bool Editor::saveFile(const std::string& path) {
    if (!initialized_ || !buffer_) return false;
    return buffer_->saveToFile(path);
}

void Editor::insertText(std::string_view text) {
    if (!initialized_ || !buffer_) return;
    const Selection& sel = buffer_->getSelection();
    if (sel.hasSelection()) {
        auto [start, end] = sel.range();
        buffer_->replace(start, end - start, text);
    } else {
        buffer_->insert(sel.start, text);
    }
}

void Editor::deleteSelection() {
    if (!initialized_ || !buffer_) return;
    const Selection& sel = buffer_->getSelection();
    if (sel.hasSelection()) {
        auto [start, end] = sel.range();
        buffer_->erase(start, end - start);
    }
}

void Editor::setSelection(size_t start, size_t end) {
    if (!initialized_ || !buffer_) return;
    buffer_->setSelection(start, end);
}

void Editor::setCursor(size_t pos) {
    if (!initialized_ || !buffer_) return;
    buffer_->setCursor(pos);
}

void Editor::cleanup() {
    if (renderer_) {
        renderer_->cleanup();
    }
    if (window_) {
        window_->close();
    }
    buffer_.reset();
    renderer_.reset();
    window_.reset();
    initialized_ = false;
}

} // namespace editor 