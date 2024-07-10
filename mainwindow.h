#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QColorDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStack>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isModified() const { return modified; }
    QColor penColor() const { return myPenColor; }
    int penWidth() const { return myPenWidth; }

    void setPenColor(const QColor &newColor);
    void setPenWidth(int newWidth);

private slots:
    void newfile();
    void open();
    void save();
    void saveAs();
    void exits();
    void penWidths();
    void penColors();
    void undo();
    void redo();
    void clear();
    void selectionArea();


    void on_actionOval_triggered();

    void on_actionRectangle_triggered();

    void on_actionLine_triggered();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    QColor myPenColor;
    QImage image;
    int myPenWidth;

    bool modified;
    bool drawing;
    bool selecting;
    bool movingSelection;
    bool drawingShape;

    enum Shape { None, Line, Rectangle, Oval };
    Shape currentShape;

    QPoint lastMousePos;
    QPoint selectionStart;
    QPoint selectionEnd;
    QRect selectionRect;
    QImage selectedImage;
    QPoint selectionOffset;

    void resizeImage(int newWidth, int newHeight);
    void saveImageState();
    void drawSelectionRect(QPainter &painter);
    void clearSelectedArea();

    QStack<QImage> undoStack;
    QStack<QImage> redoStack;
};

#endif // MAINWINDOW_H
