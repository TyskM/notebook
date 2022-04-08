#include "ToolSelector.h"

#include "Tool.h"
#include "Canvas.h"
#include "Helpers.h"
#include <qtoolbutton.h>

ToolSelector::ToolSelector(QWidget* parent, Canvas* canvas) : QWidget(parent)
{
    this->canvas = canvas;

    mainLayout->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    setLayout(mainLayout);
    mainLayout->addLayout(toolListLayout);
    mainLayout->addLayout(toolActionListLayout);

    toolActionListLayout->setAlignment(Qt::AlignTop);
    
    // HACK: 
    //QSpacerItem* spacer = new QSpacerItem(1, 38, QSizePolicy::Minimum, QSizePolicy::Maximum);
    //toolActionListLayout->addSpacerItem(spacer);
}

ToolSelector::~ToolSelector() { clearTools(); }

bool ToolSelector::containsTool(Tool& tool) const
{
    for (auto& toolButtonPair : tools)
    { if (toolButtonPair.first == &tool) return true; }
    return false;
}

void ToolSelector::addTool(Tool* tool)
{
    if (containsTool(*tool))
    {
        qDebug() << "ToolSelector already has tool named: " << tool->name;
        return;
    }

    tool->canvas = canvas;
    QPushButton* button = new QPushButton();
    connect(button, &QPushButton::clicked, this, [this, tool, button]() { onToolButtonClicked(*tool, *button); });
    button->setCheckable(true);
    button->setIconSize(iconSize);
    button->setMaximumSize(buttonSize);
    button->setToolTip(tool->name);
    button->setIcon(tool->icon);
    toolListLayout->addWidget(button);

    tools.emplace_back(tool, button);
}

void ToolSelector::removeTool(Tool& tool)
{
    if (tools.size() == 0) qDebug("removeTool called when ToolSelector tool list is empty");

    for (size_t i = 0; i < tools.size(); i++)
    {
        auto& toolButtonPair = tools[i];
        if (toolButtonPair.first == &tool)
        {
            toolListLayout->removeWidget(toolButtonPair.second);
            tools.erase(tools.begin() + i);
            delete &tool;
            return;
        }
    }
}

void ToolSelector::clearTools()
{
    for (auto& toolButtonPair : tools)
    { delete toolButtonPair.first; }
    tools.clear();
    Helpers::clearLayout(toolActionListLayout);
}

void ToolSelector::onToolButtonClicked(Tool& tool, QPushButton& button)
{
    for (auto& buttonToolPair : tools) { buttonToolPair.second->setChecked(false); }
    button.setChecked(true);
    if (&tool == canvas->currentTool) return;
    
    if (canvas->currentTool != nullptr) canvas->currentTool->onExit(toolActionListLayout);
    canvas->currentTool = &tool;
    tool.onEnter(toolActionListLayout);
}
