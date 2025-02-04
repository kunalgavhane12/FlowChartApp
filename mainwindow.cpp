#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , myPenColor(Qt::black)
    , myPenWidth(1)
    , modified(false)
    , drawing(false)
    , selecting(false)
    , movingSelection(false)
    , drawingShape(false)
    , currentShape(None)
{
    ui->setupUi(this);
    image = QImage(this->size(), QImage::Format_ARGB32);
    image.fill(Qt::white);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newfile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exits);

    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);
    connect(ui->actionClear, &QAction::triggered, this, &MainWindow::clear);

    connect(ui->actionPen_Width, &QAction::triggered, this, &MainWindow::penWidths);
    connect(ui->actionPen_Color, &QAction::triggered, this, &MainWindow::penColors);
    connect(ui->actionSelection, &QAction::triggered, this, &MainWindow::selectionArea);

    connect(ui->actionOval, &QAction::triggered, this, &MainWindow::on_actionOval_triggered);
    connect(ui->actionRectangle, &QAction::triggered, this, &MainWindow::on_actionRectangle_triggered);
    connect(ui->actionLine, &QAction::triggered, this, &MainWindow::on_actionLine_triggered);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPenColor(const QColor &newColor)
{
    myPenColor = newColor;
}

void MainWindow::setPenWidth(int newWidth)
{
    myPenWidth = newWidth;
}

void MainWindow::newfile()
{
    image.fill(Qt::white);
    modified = false;
    update();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty())
    {
        image.load(fileName);
        modified = false;
        update();
    }
}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath());
    if (!fileName.isEmpty())
    {
        image.save(fileName);
        modified = false;
    }
}

void MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath());
    if (!fileName.isEmpty())
    {
        image.save(fileName);
        modified = false;
    }
}

void MainWindow::exits()
{
    close();
}

void MainWindow::penWidths()
{
    bool ok;
    int newWidth = QInputDialog::getInt(this, tr("Pen Width"), tr("Select pen width:"), myPenWidth, 1, 50, 1, &ok);
    if (ok)
    {
        setPenWidth(newWidth);
    }
}

void MainWindow::penColors()
{
    QColor newColor = QColorDialog::getColor(myPenColor, this);
    if (newColor.isValid())
    {
        setPenColor(newColor);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (selecting)
        {
            selectionStart = event->pos();
            selectionEnd = selectionStart;
        }
        else if (movingSelection && selectionRect.contains(event->pos()))
        {
            selectionOffset = event->pos() - selectionRect.topLeft();
        }
        else if (drawingShape)
        {
            selectionStart = event->pos();
            selectionEnd = selectionStart;
            saveImageState();  // Save the state before starting to draw
        }
        else
        {
            drawing = true;
            lastMousePos = event->pos();
            saveImageState();  // Save the state before starting to draw
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (drawing)
    {
        if ((event->buttons() & Qt::LeftButton) && drawing)
        {
            QPainter painter(&image);
            painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawLine(lastMousePos, event->pos());
            modified = true;
            lastMousePos = event->pos();
            update();
        }
    }
    else if (movingSelection)
    {
        selectionRect.moveTopLeft(event->pos() - selectionOffset);
        update();
    }
    else if (selecting)
    {
        selectionEnd = event->pos();
        selectionRect = QRect(selectionStart, selectionEnd).normalized();
        update();
    }
    else if (drawingShape)
    {
        selectionEnd = event->pos();
        update();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (drawing)
        {
            drawing = false;
        }
        else if (movingSelection)
        {
            QPainter painter(&image);
            painter.drawImage(selectionRect.topLeft(), selectedImage);
            movingSelection = false;
            modified = true;
            update();
        }
        else if (selecting)
        {
            selecting = false;
            movingSelection = true;
            selectedImage = image.copy(selectionRect);
            clearSelectedArea();  // Clear the selected area from the original image
        }
        else if (drawingShape)
        {
            drawingShape = false;
            QPainter painter(&image);
            painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            QRect shapeRect = QRect(selectionStart, selectionEnd).normalized();
            if (currentShape == Rectangle)
            {
                painter.drawRect(shapeRect);
            }
            else if (currentShape == Oval)
            {
                painter.drawEllipse(shapeRect);
            }
            else if (currentShape == Line)
            {
                painter.drawLine(selectionStart, selectionEnd);
            }
            modified = true;
            update();
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        close();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect rect = event->rect();
    painter.drawImage(rect, image, rect);

    if (selecting || movingSelection || drawingShape)
    {
        drawSelectionRect(painter);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (width() > image.width() || height() > image.height())
    {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(newWidth, newHeight);
        update();
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::resizeImage(int newWidth, int newHeight)
{
    if (image.size() == QSize(newWidth, newHeight))
        return;

    QImage newImage(newWidth, newHeight, QImage::Format_ARGB32);
    newImage.fill(Qt::white);
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), image);
    image = newImage;
    modified = true;
}

void MainWindow::saveImageState()
{
    if (!redoStack.isEmpty())
    {
        redoStack.clear();
    }
    undoStack.push(image);
}

void MainWindow::undo()
{
    if (!undoStack.isEmpty())
    {
        redoStack.push(image);
        image = undoStack.pop();
        update();
    }
}

void MainWindow::redo()
{
    if (!redoStack.isEmpty())
    {
        undoStack.push(image);
        image = redoStack.pop();
        update();
    }
}

void MainWindow::drawSelectionRect(QPainter &painter)
{
    painter.setPen(QPen(Qt::DashLine));
    painter.drawRect(selectionRect);
}

void MainWindow::clear()
{
    image.fill(qRgb(255,255,255));
    modified = true;
    update();
}

void MainWindow::selectionArea()
{
    selecting = true;
}

void MainWindow::clearSelectedArea()
{
    QPainter painter(&image);
    painter.fillRect(selectionRect, Qt::white);
    update();
}

void MainWindow::on_actionOval_triggered()
{
    currentShape = Oval;
    drawingShape = true;
}

void MainWindow::on_actionRectangle_triggered()
{
    currentShape = Rectangle;
    drawingShape = true;
}

void MainWindow::on_actionLine_triggered()
{
    currentShape = Line;
    drawingShape = true;
}
