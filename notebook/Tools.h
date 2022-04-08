#pragma once

#include <qcolor.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qcolordialog.h>
#include <qinputdialog.h>
#include <vector>
#include <qaction.h>
#include <qtoolbutton.h>
#include <qspinbox.h>
#include "Tool.h"
#include "Canvas.h"
#include "Helpers.h"

// All the simple tools

struct DefaultSubButton : public QToolButton
{
    DefaultSubButton(QAction* action, QWidget* parent = nullptr) : QToolButton(parent)
    {
        setDefaultAction(action);
    }
};

class CursorTool : public Tool
{
public:
    CursorTool(QObject* parent = nullptr) : Tool(parent)
    {
        icon = QIcon("res/cursor.png");
        name = "Cursor";
    }

    virtual void onEnter(QLayout* subtoolLayout)
    {
        canvas->setFocusPolicy(Qt::ClickFocus);
        canvas->viewport()->setCursor(Qt::IBeamCursor);
    };

    virtual void onExit(QLayout* subtoolLayout)  { };
    virtual void mousePressEvent(QMouseEvent* event)   { canvas->baseMousePressEvent(event); };
    virtual void mouseMoveEvent(QMouseEvent* event)    { canvas->baseMouseMoveEvent(event); };
    virtual void mouseReleaseEvent(QMouseEvent* event) { canvas->baseMouseReleaseEvent(event); };
    virtual void keyPressEvent(QKeyEvent* event)       { canvas->baseKeyPressEvent(event); }
};

class DrawTool : public Tool
{
protected:
    const int minimumCursorSize = 3; // The cursor is hard to see below 3
    bool drawing = false;
    bool erasing = false;
    int penWidth = 1;
    QColor penColor = Qt::black;
    QPoint lastPoint;

    QAction* setColorAction;
    QAction* setSizeAction;
    QAction* toggleEraserAction;

    // Need to keep track of this bc its a toggle
    QAbstractButton* eraseButton = nullptr;

public:
    DrawTool(QObject* parent = nullptr) : Tool(parent)
    {
        icon = QIcon("res/drawing.png");
        name = "Draw";

        setColorAction = new QAction(QIcon("res/color.png"), "Color", this);
        connect(setColorAction, &QAction::triggered, this, &DrawTool::penColorPrompt);

        setSizeAction = new QAction(QIcon("res/brushsize.png"), "Size", this);
        connect(setSizeAction, &QAction::triggered, this, &DrawTool::penWidthPrompt);

        toggleEraserAction = new QAction(QIcon("res/eraser.png"), "Eraser", this);
        connect(toggleEraserAction, &QAction::triggered, this, &DrawTool::toggleEraser);
    }

    void   inline setPenColor(const QColor& newColor) { penColor = newColor; }
    void   inline setPenWidth(int newWidth) { penWidth = newWidth; updateCursor(); }
    QColor inline getPenColor() const { return penColor; }
    int    inline getPenWidth() const { return penWidth; }

    // TODO: Make this a not-prompt?
    void penColorPrompt()
    {
        QColor newColor = QColorDialog::getColor(getPenColor());
        if (newColor.isValid()) setPenColor(newColor);
    }

    void penWidthPrompt()
    {
        bool ok;
        int newWidth = QInputDialog::getInt(canvas, "Pen Width", "Select pen width:", getPenWidth(), 1, 50, 1, &ok);
        if (ok) setPenWidth(newWidth);
    }

    void toggleEraser()
    {
        erasing = !erasing;
        if (erasing) eraseButton->setChecked(true);
        else         eraseButton->setChecked(false);
    }

    void drawLineTo(const QPoint& endPoint)
    {
        QPainter painter(&canvas->image);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        if (erasing) painter.setPen(QPen(Qt::transparent, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        else         painter.setPen(QPen(penColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        painter.drawLine(lastPoint, endPoint);
        canvas->modified = true;

        int rad = (penWidth / 2) + 2;
        const QRect updateRegion = QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad);
        canvas->update(updateRegion);
        lastPoint = endPoint;
    }

    void inline onBrushSizeWidgetValueChanged(int value) { setPenWidth(value); }

    void onEnter(QLayout* subtoolLayout) final override
    {
        auto parent = subtoolLayout->parentWidget();

        subtoolLayout->addWidget(new DefaultSubButton(setColorAction, parent));

        auto spinBox = new QSpinBox(parent);
        spinBox->setMinimum(1);
        spinBox->setMaximum(1000);
        spinBox->connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, &DrawTool::onBrushSizeWidgetValueChanged);
        spinBox->setValue(4);
        subtoolLayout->addWidget(spinBox);

        eraseButton = new DefaultSubButton(toggleEraserAction, parent);
        eraseButton->setCheckable(true);
        subtoolLayout->addWidget(eraseButton);

        canvas->setFocusPolicy(Qt::NoFocus);
        updateCursor();
    }

    void onExit(QLayout* subtoolLayout) final override
    {
        Helpers::clearLayout(subtoolLayout);
    }

    void mousePressEvent(QMouseEvent* event) final override
    {
        if (event->button() == Qt::LeftButton)
        { lastPoint = event->pos(); drawing = true; }
    }

    void mouseMoveEvent(QMouseEvent* event) final override
    {
        if ((event->buttons() & Qt::LeftButton) && drawing)
        {
            drawLineTo(event->pos());
            canvas->update();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) final override
    {
        if (event->button() == Qt::LeftButton && drawing)
        { drawLineTo(event->pos()); }
    }

    void updateCursor()
    {
        int cursorWidth = penWidth;
        if (cursorWidth < minimumCursorSize) cursorWidth = minimumCursorSize;

        QPixmap pixmap(QSize(cursorWidth, cursorWidth));
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        QRect r(QPoint(), pixmap.size());
        r.adjust(1, 1, -1, -1);
        painter.drawEllipse(r);
        painter.end();
        QCursor cursor(pixmap);
        canvas->viewport()->setCursor(cursor);
    }
};

class ShapeTool : public Tool
{
public:
    enum class Shape
    {
        rect,
        ellipse,
        line
    };

    QPoint   p1;
    QPoint   p2;
    bool     drawing = false;
    Shape    selectedShape = Shape::rect;
    QPainter painter;
    QPen     pen;

    QAction* setColor;
    QAction* selectRect;
    QAction* selectEllipse;
    QAction* selectLine;

    ShapeTool(QObject* parent = nullptr) : Tool(parent)
    {
        icon = QIcon("res/shapes.png");
        name = "Shapes";
        
        pen.setColor(Qt::black);
        pen.setWidth(1);

        setColor = new QAction(QIcon("res/color.png"), "Color", this);
        connect(setColor, &QAction::triggered, [this]()
            {
                QColor newColor = QColorDialog::getColor(pen.color());
                if (newColor.isValid()) pen.setColor(newColor);
            });

        selectRect = new QAction(QIcon("res/rect.png"), "Rectangle", this);
        selectRect->connect(selectRect, &QAction::triggered, [this]() { selectedShape = Shape::rect; });

        selectEllipse = new QAction(QIcon("res/ellipse.png"), "Ellipse", this);
        selectEllipse->connect(selectEllipse, &QAction::triggered, [this]() { selectedShape = Shape::ellipse; });

        selectLine = new QAction(QIcon("res/line.png"), "Line", this);
        selectLine->connect(selectLine, &QAction::triggered, [this]() { selectedShape = Shape::line; });
    }

    // Only use preview in paintEvent
    void drawCurrentShape(const QPoint& p1, const QPoint& p2, bool preview = false)
    {
        if (preview) painter.begin(canvas->viewport());
        else         painter.begin(&canvas->image);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setPen(pen);

        switch (selectedShape)
        {
        case ShapeTool::Shape::rect:    painter.drawRect(QRect(p1, p2));    break;
        case ShapeTool::Shape::ellipse: painter.drawEllipse(QRect(p1, p2)); break;
        case ShapeTool::Shape::line:    painter.drawLine(QLine(p1, p2));    break;
        default: break;
        }
        painter.end();
    }

    virtual void onEnter(QLayout* subtoolLayout) final override
    {
        canvas->setFocusPolicy(Qt::NoFocus);
        canvas->viewport()->setCursor(QCursor(Qt::CursorShape::CrossCursor));
        canvas->setContextMenuPolicy(Qt::NoContextMenu);

        auto parent = subtoolLayout->parentWidget();

        subtoolLayout->addWidget(new DefaultSubButton(setColor, parent));

        auto spinBox = new QSpinBox(parent);
        spinBox->setMinimum(1);
        spinBox->setMaximum(1000);
        spinBox->connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, &ShapeTool::onPenWidthWidgetValueChanged);
        spinBox->setValue(1);
        subtoolLayout->addWidget(spinBox);

        subtoolLayout->addWidget(new DefaultSubButton(selectRect, parent));
        subtoolLayout->addWidget(new DefaultSubButton(selectEllipse, parent));
        subtoolLayout->addWidget(new DefaultSubButton(selectLine, parent));
    }

    virtual void onExit(QLayout* subtoolLayout) final override
    {
        canvas->setContextMenuPolicy(Qt::DefaultContextMenu);
        Helpers::clearLayout(subtoolLayout);
    }

    virtual void mousePressEvent(QMouseEvent* event) final override
    {
        if (!canvas->isActiveWindow()) return;
        if (event->buttons() & Qt::RightButton)
        { drawing = false; }
        else if (event->buttons() & Qt::LeftButton)
        {
            if (!drawing)
            { p1 = event->pos(); p2 = event->pos(); drawing = true; }
            else
            { drawCurrentShape(p1, event->pos()); drawing = false; }
        }
    }

    virtual void mouseMoveEvent(QMouseEvent* event) final override
    {
        if (drawing) p2 = event->pos();
    }

    virtual void mouseReleaseEvent(QMouseEvent* event) final override { }

    virtual void paintEvent(QPaintEvent* event)
    {
        if (drawing)
        { drawCurrentShape(p1, p2, true); }
    }

    void onPenWidthWidgetValueChanged(int value) { pen.setWidth(value); }
};

class TextTool : public Tool
{
public:
    QString tempText;
    bool typing = false;
    int fontSize = 12;

    bool drawingRect = false;
    QPoint p1;
    QPoint p2;

    QPainter painter;
    QPen pen;

    QAction* setColor;

    TextTool(QObject* parent = nullptr) : Tool(parent)
    {
        name = "Text";
        icon = QIcon("res/text.png");

        setColor = new QAction(QIcon("res/color.png"), "Color", this);
        connect(setColor, &QAction::triggered, [this]()
            {
                QColor newColor = QColorDialog::getColor(pen.color());
                if (newColor.isValid()) pen.setColor(newColor);
            });

        tempText.reserve(512);
    }

    // Only use preview in paintEvent
    void drawText(const QPoint& p1, const QPoint& p2, bool preview = false)
    {
        if (preview) painter.begin(canvas->viewport());
        else         painter.begin(&canvas->image);
        painter.setPen(pen);
        
        auto font = painter.font();
        font.setPointSize(fontSize);
        painter.setFont(font);

        QTextOption qto;
        // TODO: add alignment and wrap buttons one day zzzzzzz
        qto.setWrapMode(QTextOption::WordWrap);
        qto.setAlignment(Qt::AlignCenter);
        painter.drawText(QRect(p1, p2), tempText, qto);

        painter.end();
    }
    
    void drawPreviewRect()
    {
        painter.begin(canvas->viewport());
        painter.setPen(pen);
        painter.drawRect(QRect(p1, p2));
        painter.end();
    }

    void finalizeText()
    {
        if (!typing) return;
        drawText(p1, p2);
        typing = false;
        tempText.clear();
    }

    void cancelText()
    {
        tempText.clear();
        typing = false;
    }

    void inline onTextSizeWidgetValueChanged(int value) noexcept { fontSize = value; }

    virtual void onEnter(QLayout* subtoolLayout) override
    {
        canvas->setFocusPolicy(Qt::ClickFocus);
        canvas->viewport()->setCursor(QCursor(Qt::CursorShape::CrossCursor));
        auto parent = subtoolLayout->parentWidget();

        subtoolLayout->addWidget(new DefaultSubButton(setColor, parent));

        auto spinBox = new QSpinBox(parent);
        spinBox->setMinimum(1);
        spinBox->setMaximum(1000);
        spinBox->connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, &TextTool::onTextSizeWidgetValueChanged);
        spinBox->setValue(12);
        subtoolLayout->addWidget(spinBox);
    }

    virtual void onExit(QLayout* subtoolLayout) override  { Helpers::clearLayout(subtoolLayout); }

    virtual void mousePressEvent(QMouseEvent* event) override
    {
        if (typing) { finalizeText(); drawingRect = false; return; }

        if (event->buttons() == Qt::RightButton) { drawingRect = false; typing = false; }
        else if (event->buttons() == Qt::LeftButton)
        {
            if (!drawingRect)
            { p1 = event->pos(); p2 = event->pos(); drawingRect = true; typing = false; }
            else
            { drawingRect = false; typing = true; }
        }
    }

    virtual void mouseMoveEvent(QMouseEvent* event) override
    {
        if (drawingRect) p2 = event->pos();
    }

    virtual void mouseReleaseEvent(QMouseEvent* event) override { }

    virtual void keyPressEvent(QKeyEvent* event) override
    {
        if (typing)
        {
            if (event->key() == Qt::Key::Key_Escape)
            { cancelText(); }
            else if (event->key() == Qt::Key::Key_Backspace)
            { tempText.chop(1); }
            else if (event->key() == Qt::Key::Key_Return || event->key() == Qt::Key::Key_Enter)
            { tempText.append("\n"); }
            else { tempText.append(event->text()); } 
        }
        else
        { canvas->baseKeyPressEvent(event); }
    }

    virtual void paintEvent(QPaintEvent* event) override
    {
        if (drawingRect || typing) drawPreviewRect();
        if (typing) drawText(p1, p2, true);
    }
};