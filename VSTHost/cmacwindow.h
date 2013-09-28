#ifndef CMACWINDOW_H
#define CMACWINDOW_H

#include <QtCore>
#include <QHideEvent>
#include <QMacCocoaViewContainer>

class CMacWindow : public QMacCocoaViewContainer
{
    Q_OBJECT
public:
    CMacWindow(QWidget* parent=0);
    ~CMacWindow();
protected:
    void hideEvent(QHideEvent *e);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void DestroyMacWindow();
    void Init();
    void* WindowReference();
    void * cocoaWin;
private:
    void CreateMacWindow();
    void* hostView;
};

#endif // CMACWINDOW_H
