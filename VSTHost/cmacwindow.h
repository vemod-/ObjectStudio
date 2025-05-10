#ifndef CMACWINDOW_H
#define CMACWINDOW_H

#include <QtCore>
#include <QWidget>

class CMacWindow : public QWidget
{
    Q_OBJECT
public:
    CMacWindow(QWidget* parent=0);
    ~CMacWindow();
    //void setWidgetSize(const QSize &s);
    void* viewId() const;
    void* superId() const;
    void setViewSize(QSize s);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QPixmap grab(const QRect &rectangle = QRect(QPoint(0, 0), QSize(-1, -1)));
protected:
    void init(void* host = nullptr);
    void destroyMacWindow();
    //void releaseMacWindow();
    //bool event(QEvent* e);
private:
    void createMacWindow(void* host);
    const QRect geometryOnParentWindow() const;
    WId parentWindowNumber() const;
    QWidget* w = nullptr;
    //void* hostView = nullptr;
    QTimer sizeTimer;
    //void* cocoaViewObserver = nullptr;
    //void* superViewObserver = nullptr;
public slots:
    void checkGeometry();
};

#endif // CMACWINDOW_H
