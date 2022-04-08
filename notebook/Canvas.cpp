#include "Canvas.h"
#include "Tool.h"

Canvas::Canvas(QWidget* parent) : QTextEdit::QTextEdit(parent)
{
    
}

bool Canvas::save(const QString& filePath)
{
    using OpenFlags = QIODevice::OpenModeFlag;
    QuaZip saveZip(filePath);
    saveZip.open(QuaZip::mdCreate);

    // Write image
    QImage visibleImage = image;
    resizeImage(&visibleImage, size());

    QByteArray imgba;
    QBuffer imgbuffer(&imgba);
    imgbuffer.open(QIODevice::WriteOnly);
    visibleImage.save(&imgbuffer, "png"); // writes image into ba in PNG format

    QuaZipFile imgFile(&saveZip);
    imgFile.open(OpenFlags::WriteOnly, QuaZipNewInfo("image.png"));
    imgFile.write(imgba);
    imgFile.close();
    imgbuffer.close();

    // Write text
    QByteArray textba = toPlainText().toUtf8();
    QuaZipFile textFile(&saveZip);
    textFile.open(OpenFlags::WriteOnly, QuaZipNewInfo("text.txt"));
    textFile.write(textba);
    textFile.close();

    saveZip.close();

    if (saveZip.getZipError() == UNZ_OK)
    {
        modified = false;
        return true;
    }
    return false;
}

bool Canvas::load(const QString& filePath)
{
    using OpenFlags = QIODevice::OpenModeFlag;
    QuaZip loadZip(filePath);
    loadZip.open(QuaZip::mdUnzip);

    // Read img
    loadZip.setCurrentFile("image.png");
    QuaZipFile imgFile(&loadZip);
    imgFile.open(OpenFlags::ReadOnly);
    QImage img;
    img.loadFromData(imgFile.readAll());
    setImage(img);
    imgFile.close();

    // Read text
    loadZip.setCurrentFile("text.txt");
    QuaZipFile textFile(&loadZip);
    textFile.open(OpenFlags::ReadOnly);
    setText(QString(textFile.readAll()));
    textFile.close();

    loadZip.close();

    modified = false;
    return true;
}

bool Canvas::setImageFromPath(const QString& path)
{
    QImage loadedImage;
    if (!loadedImage.load(path)) return false;
    setImage(loadedImage);
    return true;
}

void Canvas::setImage(const QImage& newImg)
{
    QSize newSize = newImg.size().expandedTo(size());
    image = newImg;
    resizeImage(&image, newSize);
    modified = false;
    update();
}

bool Canvas::exportImg(const QString& filePath, const char* fileFormat)
{
    QImage visibleImage = image;
    resizeImage(&visibleImage, size());

    if (visibleImage.save(filePath, fileFormat)) { return true; }
    return false;
}

void Canvas::mousePressEvent(QMouseEvent* event)
{ if (currentTool != nullptr) currentTool->mousePressEvent(event); }

void Canvas::mouseMoveEvent(QMouseEvent* event)
{ if (currentTool != nullptr) currentTool->mouseMoveEvent(event); }

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{ if (currentTool != nullptr) currentTool->mouseReleaseEvent(event); }

void Canvas::paintEvent(QPaintEvent* event)
{
    QPainter painter(viewport());
    QRect dirtyRect = event->rect();
    painter.drawImage(dirtyRect, image, dirtyRect);
    QTextEdit::paintEvent(event);
    if (currentTool != nullptr) currentTool->paintEvent(event);
    viewport()->update();
}

void Canvas::resizeEvent(QResizeEvent* event)
{
    QTextEdit::resizeEvent(event);
    if (width() > image.width() || height() > image.height())
    {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
        update();
    }
}

void Canvas::keyPressEvent(QKeyEvent* event)
{
    currentTool->keyPressEvent(event);
    if (event->modifiers() & Qt::ControlModifier)
    {
        // TODO: Setup hotkeys (move to tool selector ig)
        //if      (event->key() == Qt::Key_Q) { enterBasicMode(); }
        //else if (event->key() == Qt::Key_W) { enterDrawMode(); }
    }
}

void Canvas::resizeImage(QImage* image, const QSize& newSize)
{
    if (image->size() == newSize)
        return;

    QImage newImage(newSize, QImage::Format_ARGB32);
    newImage.fill(Qt::transparent);
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0),* image);
    *image = newImage;
}
