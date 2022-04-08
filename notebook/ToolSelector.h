#pragma once

#include <qlayout.h>
#include <qpushbutton.h>
#include <utility>
#include <vector>
#include <qdebug.h>

class Canvas;
class Tool;

class ToolSelector : public QWidget
{
    Q_OBJECT

public:
    std::vector<std::pair<Tool*, QPushButton*>> tools;

    const QSize iconSize   { 32, 32 };
    const QSize buttonSize { 42, 42 };

    Canvas* canvas = nullptr;
    QVBoxLayout* mainLayout           = new QVBoxLayout();
    QHBoxLayout* toolListLayout       = new QHBoxLayout();
    QHBoxLayout* toolActionListLayout = new QHBoxLayout();

    ToolSelector(QWidget* parent, Canvas* canvas);
    ~ToolSelector();
    bool containsTool(Tool& tool) const;
    void addTool(Tool* tool); // Takes ownership
    void removeTool(Tool& tool);
    void clearTools();

protected:
    void onToolButtonClicked(Tool& tool, QPushButton& button);
};