#include "cthreadedfunction.h"

CSpinLabel::CSpinLabel(QWidget* parent)
    : QObject(parent) {
    l = new QLabel(parent);
    QRect r;
    if (parent) {
        r = QRect(parent->mapToGlobal(parent->mapFromParent(parent->geometry().topLeft())),parent->geometry().size());
        parent->installEventFilter(this);
    }
    CConcurrentDialog::showSpinner(l,true,r);
}

CSpinLabel::~CSpinLabel() {
    l->deleteLater();
}

bool CSpinLabel::eventFilter(QObject *obj, QEvent *event) {
    QEvent::Type type=event->type();
    if (type == QEvent::Resize) {
        GeometryChanged();
    }
    return QObject::eventFilter(obj, event);
}

void CSpinLabel::GeometryChanged() {
    l->setFixedSize(l->parentWidget()->geometry().size());
}

