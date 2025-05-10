#ifndef CTIMELINEEDIT_H
#define CTIMELINEEDIT_H

#include <QWidget>
#include <QMenu>
#include "ctimeline.h"

class CTimeLineMenu : public QMenu {
    Q_OBJECT
public:
    CTimeLineMenu(CTimeLine* t, QWidget* parent);
signals:
    void Changed();
};

namespace Ui {
class CTimeLineEdit;
}

class CTimeLineEdit : public QWidget
{
    Q_OBJECT
public:
    ~CTimeLineEdit();
    explicit CTimeLineEdit(CTimeLine* t, QWidget* parent);
    void hideEvent(QHideEvent* /*event*/);
signals:
    void Changed();
private:
    Ui::CTimeLineEdit *ui;
    CTimeLine* m_TimeLine;
};

#endif // CTIMELINEEDIT_H
