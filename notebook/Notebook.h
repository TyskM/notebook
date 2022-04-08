
/*
    Tyler Tucker
    2022/03/12
    Drawing/Notepad app

    This example from the QT docs was used as a base:
    https://doc.qt.io/qt-5/qtwidgets-widgets-scribble-example.html

    Features:
    Drawing and text editing inside a single widget.
    Saving and loading to and from zip files, which allows for easily reading/modifying the contents in other programs.
    Exporting the image to many formats

    TODO:
    Save to file (done)
    Load from file (done)
    Add icons to the tool selector (done)
    Add brush cursor (done)
    Add eraser (done)
    Importing and transforming images
    Reminders feature
    text to image for export
*/

#pragma once

#include <QtWidgets/QMainWindow>
#include <qdir.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qpainter.h>
#include <qimagewriter.h>
#include <qaction.h>
#include <qcolordialog.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qtextedit.h>
#include <qpaintengine.h>
#include <qpushbutton.h>
#include <qboxlayout.h>
#include <qiodevice.h>

#include "ToolSelector.h"
#include "Canvas.h"
#include "Tools.h"

class Notebook : public QMainWindow
{
    Q_OBJECT

public:
    const QString appName = "Notebook";
    const QString customSaveFileFormat = "nb";

    QWidget*     root;
    QVBoxLayout* rootLayout;

    Canvas* canvas;
    ToolSelector* toolSelector;

    QMenu* exportAsMenu;
    QMenu* fileMenu;
    QMenu* optionMenu;
    QMenu* helpMenu;

    QAction* saveAct;
    QAction* loadAct;
    QAction* openAct;
    QList<QAction*> exportAsActs;
    QAction* exitAct;
    QAction* penColorAct;
    QAction* penWidthAct;
    QAction* clearScreenAct;
    QAction* aboutAct;

    Notebook(QWidget* parent = Q_NULLPTR);
    void buildActionMenu();
    void closeEvent(QCloseEvent* event) override;
    bool trySave();
    void about();
    void openFile();
    bool load();
    bool save();
    void exportAction();
    bool exportToImg(const QByteArray& fileFormat);
};
