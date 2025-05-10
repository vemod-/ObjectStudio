#ifndef CMASTERVOL_H
#define CMASTERVOL_H

#include <QFrame>

namespace Ui {
class CMasterVol;
}

class CMasterVol : public QFrame
{
    Q_OBJECT

public:
    explicit CMasterVol(QWidget *parent = 0);
    ~CMasterVol();
    int leftVol();
    int rightVol();
    bool lock();
protected:
    void showEvent(QShowEvent* e);
signals:
    void leftVolChanged(int v);
    void rightVolChanged(int v);
    void lockChanged(bool v);
public slots:
    void setLeftVol(int v);
    void setRightVol(int v);
    void setLock(bool v);
    void peak(float l, float r);
    void resetPeak();
private:
    Ui::CMasterVol *ui;
};

#endif // CMASTERVOL_H
