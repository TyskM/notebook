#pragma once

#include <qobject.h>
#include <qevent.h>
#include <qaction.h>
#include <vector>

class Canvas;

class Tool : public QObject
{
public:
    Canvas* canvas = nullptr;
    QString name   = "Tool";
    QIcon   icon;

    Tool(QObject* parent = nullptr) : QObject(parent) { }
    virtual ~Tool() = default;

    virtual void onEnter(QLayout* subtoolLayout)       { }
    virtual void onExit(QLayout* subtoolLayout)        { }
    virtual void mousePressEvent(QMouseEvent* event)   { }
    virtual void mouseMoveEvent(QMouseEvent* event)    { }
    virtual void mouseReleaseEvent(QMouseEvent* event) { }
    virtual void keyPressEvent(QKeyEvent* event)       { }
    virtual void paintEvent(QPaintEvent* event)        { }

};
