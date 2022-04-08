#include "Notebook.h"

Notebook::Notebook(QWidget* parent)
{
    setWindowTitle(appName);
    resize(800, 600);

    root         = new QWidget(this);
    rootLayout   = new QVBoxLayout(root);
    canvas       = new Canvas(root);
    toolSelector = new ToolSelector(root, canvas);

    toolSelector->addTool(new CursorTool(toolSelector));
    toolSelector->addTool(new DrawTool  (toolSelector));
    toolSelector->addTool(new ShapeTool (toolSelector));
    toolSelector->addTool(new TextTool  (toolSelector));

    setCentralWidget(root);
    root->setLayout(rootLayout);
    rootLayout->addWidget(toolSelector);
    rootLayout->addWidget(canvas);

    buildActionMenu();

    show();
}

void Notebook::buildActionMenu()
{
    saveAct = new QAction("&Save Project...", this);
    saveAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    connect(saveAct, &QAction::triggered, this, &Notebook::save);

    loadAct = new QAction("&Load Project...");
    connect(loadAct, &QAction::triggered, this, &Notebook::load);

    openAct = new QAction("&Import Image...", this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &Notebook::openFile);

    const QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();
    for (const QByteArray &format : imageFormats) {
        QString text = tr("%1...").arg(QString::fromLatin1(format).toUpper());

        QAction* action = new QAction(text, this);
        action->setData(format);
        connect(action, &QAction::triggered, this, &Notebook::exportAction);
        exportAsActs.append(action);
    }

    exitAct = new QAction("E&xit", this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &Notebook::close);

    clearScreenAct = new QAction("&Clear Screen", this);
    clearScreenAct->setShortcut(tr("Ctrl+L"));
    connect(clearScreenAct, &QAction::triggered, canvas, &Canvas::clearImage);

    aboutAct = new QAction("&About", this);
    connect(aboutAct, &QAction::triggered, this, &Notebook::about);

    exportAsMenu = new QMenu("&Export", this);
    for (QAction* action : qAsConst(exportAsActs)) exportAsMenu->addAction(action);

    fileMenu = new QMenu("&File", this);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(loadAct);
    fileMenu->addAction(openAct);
    fileMenu->addMenu(exportAsMenu);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    optionMenu = new QMenu("&Edit", this);
    optionMenu->addAction(clearScreenAct);

    helpMenu = new QMenu("&Help", this);
    helpMenu->addAction(aboutAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(optionMenu);
    menuBar()->addMenu(helpMenu);
}

void Notebook::closeEvent(QCloseEvent* event)
{
    if (trySave()) event->accept();
    else event->ignore();
}

bool Notebook::trySave()
{
    if (canvas->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, appName,
            "The image has been modified.\n"
            "Do you want to save your changes?",
            QMessageBox::Save | QMessageBox::Discard
            | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) return exportToImg("png");
        else if (ret == QMessageBox::Cancel) return false;
    }
    return true;
}

void Notebook::about()
{
    QMessageBox::about(this, "About " + appName, "<p> 2022 Tyler Tucker </p>");
}

void Notebook::openFile()
{
    if (trySave())
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Open File", QDir::currentPath());
        if (!fileName.isEmpty()) canvas->setImageFromPath(fileName);
    }
}

bool Notebook::load()
{
    QString initialPath = QDir::currentPath();
    QString fileName = QFileDialog::getOpenFileName(this, "Load", initialPath, appName + " Files (*." + customSaveFileFormat + ");;All Files (*)");
    if (fileName.isEmpty()) return false;
    canvas->load(fileName);
    return true;
}

bool Notebook::save()
{
    QString initialPath = QDir::currentPath() + "/untitled." + customSaveFileFormat;
    QString fileName = QFileDialog::getSaveFileName(this, "Save as", initialPath, appName + " Files (*." + customSaveFileFormat + ");;All Files (*)");
    if (fileName.isEmpty()) return false;
    canvas->save(fileName);
    return true;
}

void Notebook::exportAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    QByteArray fileFormat = action->data().toByteArray();
    exportToImg(fileFormat);
}

bool Notebook::exportToImg(const QByteArray& fileFormat)
{
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;
    QString fileName = QFileDialog::getSaveFileName(this, "Export As", initialPath, "%1 Files (*.%2);;All Files (*)");
    if (fileName.isEmpty()) return false;
    canvas->exportImg(fileName, fileFormat.constData());
    return true;
}