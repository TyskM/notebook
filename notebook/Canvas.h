#pragma once

#include <qtextedit.h>
#include <qpainter.h>
#include <qevent.h>
#include <vector>

// For saving
#include <QuaZip-Qt5-1.1/quazip/quazip.h>
#include <QuaZip-Qt5-1.1/quazip/quazipfile.h>
#include <qbuffer.h>

class Tool;

class Canvas : public QTextEdit
{
    Q_OBJECT

public:
    Tool* currentTool = nullptr;
    bool modified = false;
    QImage image;

    Canvas(QWidget* parent = nullptr);
    bool save(const QString& filePath);
    bool load(const QString& filePath);
    bool setImageFromPath(const QString& path);
    void setImage(const QImage& newImg);
    bool exportImg(const QString& filePath, const char* fileFormat);

    void mousePressEvent(QMouseEvent* event)   override;
    void mouseMoveEvent(QMouseEvent* event)    override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event)        override;
    void resizeEvent(QResizeEvent* event)      override;
    void keyPressEvent(QKeyEvent* event)       override;
    void resizeImage(QImage* image, const QSize& newSize);

    void inline baseMousePressEvent(QMouseEvent* event)   { QTextEdit::mousePressEvent(event); };
    void inline baseMouseMoveEvent(QMouseEvent* event)    { QTextEdit::mouseMoveEvent(event); };
    void inline baseMouseReleaseEvent(QMouseEvent* event) { QTextEdit::mouseReleaseEvent(event); };
    void inline baseKeyPressEvent(QKeyEvent* event)       { QTextEdit::keyPressEvent(event); }
    bool inline isModified()  const { return modified; }

public slots:
    void clearImage()
    {
        image.fill(qRgba(255, 255, 255, 0));
        modified = true;
        update();
    }
};