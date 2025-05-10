#include "csoftsynthsform.h"
#include "idevice.h"
#include "qdomlite.h"
//#include <QPainter>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>
#ifndef __x86_64
//#include <Carbon/Carbon.h>
#endif

CParameterWrapper::CParameterWrapper(CParameter* p, QObject* parent) :
    QObject(parent)
{
    //m_SignalControl = nullptr;
    //m_SlotControl = nullptr;
    m_OwnerParameter = p;
    //m_ControlOffset = 0;
    p->setControl(this);
}

CParameterWrapper::~CParameterWrapper()
{
    disconnect(this);
    m_SignalControl = nullptr;
    m_SlotControl = nullptr;
}

void CParameterWrapper::setOffset(int v)
{
    m_ControlOffset = v;
}

void CParameterWrapper::updateParameter(const CParameter* p)
{
    if (m_SlotControl)
    {
        QMutexLocker locker(&mutex);
        m_SlotControl->blockSignals(true);
        emit intValueChanged(p->Value - m_ControlOffset);
        m_SlotControl->blockSignals(false);
    }
}

void CParameterWrapper::setValue(int v)
{
    m_OwnerParameter->setValue(v);
}

void CParameterWrapper::setIntValue(int v)
{
    m_OwnerParameter->setValue(v + m_ControlOffset);
}

void CParameterWrapper::setPercentValue(float v)
{
    m_OwnerParameter->setPercentValue(v);
}

void CParameterWrapper::setdBValue(float v)
{
    m_OwnerParameter->setdBValue(v);
}

void CParameterWrapper::setBoolValue(bool v)
{
    m_OwnerParameter->setValue(v);
}

#ifdef Q_OS_IOS
#include <QHBoxLayout>

customSizeCorner::customSizeCorner(QWidget* parent, QWidget* form)
    : QDialog(parent,Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint)
{
    setAttribute(Qt::WA_MacAlwaysShowToolWindow);
    parentForm = form;
    setFixedSize(16,16);
    setStyleSheet("background-color: black;");
    setAutoFillBackground(true);

    cornerLabel = new QLabel("");
    cornerLabel->setStyleSheet("background-color: black;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(cornerLabel);
}

void customSizeCorner::mousePressEvent(QMouseEvent *event) {
    clickPosition = event->globalPosition().toPoint() - geometry().topLeft();
    QMouseEvent pressEvent(QEvent::NonClientAreaMouseButtonPress, mapFromGlobal(event->globalPosition()),event->button(),event->buttons(),event->modifiers());
    QApplication::sendEvent(parentForm,&pressEvent);
}

void customSizeCorner::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        qDebug() << QPoint(event->globalPosition().toPoint() - clickPosition).y();
        QPoint p = event->globalPosition().toPoint() - clickPosition;
        this->move(event->globalPosition().toPoint() - clickPosition);
        parentForm->setGeometry(parentForm->geometry().left(),parentForm->geometry().top(),p.x()-parentForm->geometry().left()+10,p.y()-parentForm->geometry().top()+10);
        parentForm->updateGeometry();
        this->move(parentForm->geometry().bottomRight() - QPoint(10,10));
    }
}

void customSizeCorner::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    parentForm->activateWindow();
}

void customSizeCorner::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    qDebug() << "customSizeCorner showEvent";
    move(parentForm->geometry().bottomRight()-QPoint(10,10));
}

CustomTitleBar::CustomTitleBar(QWidget *parent,QWidget* form, QString label, bool fixedSize)
    : QDialog(parent,Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint)
{
    setAttribute(Qt::WA_MacAlwaysShowToolWindow);
    setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::TapAndHoldGesture);
    parentForm = form;
    setFixedHeight(16);
    setStyleSheet("background-color: black;");

    setAutoFillBackground(true);

    titleLabel = new QLabel(label);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: white; background-color: black;");
    closeButton = new QPushButton("");
    closeButton->setStyleSheet("color: red; background-color: black; border-radius: 5px; border: 3px solid red;");
    closeButton->setFixedSize(14, 14);

    connect(closeButton, &QPushButton::clicked, this, &CustomTitleBar::closeRequested);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(closeButton);
    layout->addWidget(titleLabel);
    layout->setStretch(1,100);

    if (!fixedSize) sizeCorner = new customSizeCorner(parent,form);
}

void CustomTitleBar::mousePressEvent(QMouseEvent *event) {
    clickPosition = event->globalPosition().toPoint() - geometry().topLeft();
    qDebug() << "mousePress";
    setZOrder();
    QMouseEvent pressEvent(QEvent::NonClientAreaMouseButtonPress, mapFromGlobal(event->globalPosition()),event->button(),event->buttons(),event->modifiers());
    QApplication::sendEvent(parentForm,&pressEvent);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        qDebug() << QPoint(event->globalPosition().toPoint() - clickPosition).y();
        if (QPoint(event->globalPosition().toPoint() - clickPosition).y() >= 16) {
            this->move(event->globalPosition().toPoint() - clickPosition);
            parentForm->move(geometry().topLeft() + QPoint(0,16));
            updateGeometry();
            parentForm->updateGeometry();
            if (sizeCorner) {
                sizeCorner->move(parentForm->geometry().bottomRight()-QPoint(10,10));
                sizeCorner->updateGeometry();
            }
        }
    }
}

bool CustomTitleBar::event(QEvent *event) {
    if (event->type() == QEvent::Gesture) {
        qDebug() << "event gesture";
        return gestureEvent(static_cast<QGestureEvent*>(event));
    }
    return QWidget::event(event);
}

bool CustomTitleBar::gestureEvent(QGestureEvent *event) {
    qDebug() << "gesture event";
    if (QGesture *gesture = event->gesture(Qt::TapAndHoldGesture)) {
        qDebug() << "Long press detected!";
        //emit longPressDetected();  // Skicka en signal om du vill använda den
        QMouseEvent pressEvent(QEvent::NonClientAreaMouseButtonPress, mapFromGlobal(gesture->hotSpot()), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
        QApplication::sendEvent(parentForm,&pressEvent);
        return true;
    }
    return false;
}

void CustomTitleBar::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    qDebug() << "CustomTitlebar ShowEvent";
    if (parentForm->geometry().top() < 32) {
        parentForm->move(parentForm->geometry().left(),32);
    }
    //parentForm->updateGeometry();
    move(parentForm->geometry().topLeft() - QPoint(0,16));
    //updateGeometry();
    if (sizeCorner) {
        sizeCorner->move(parentForm->geometry().bottomRight()-QPoint(10,10));
        //sizeCorner->updateGeometry();
    }
}

void CustomTitleBar::setVisible(bool v) {
    QWidget::setVisible(v);
    if (sizeCorner) {
        sizeCorner->setVisible(v);
        sizeCorner->move(parentForm->geometry().bottomRight()-QPoint(10,10));
    }
}

void CustomTitleBar::setZOrder() {
    if (sizeCorner) {
        setWindowInFrontOf(sizeCorner,parentForm);
        setWindowInFrontOf(this,sizeCorner);
        setWindowInFrontOf(parentForm,this);
    }
    else {
        setWindowInFrontOf(this,parentForm);
        setWindowInFrontOf(parentForm,this);
    }
}
#endif

CSoftSynthsForm::CSoftSynthsForm(IDevice* Device, const bool FixedSize, QWidget *parent)
    : QDialog(parent,Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint)
{
    setAttribute(Qt::WA_MacAlwaysShowToolWindow);
    m_Device=Device;
    m_FixedSize=FixedSize;
    m_TimerID=0;
    for (int i = 0; i < m_Device->parameterCount(); i++) parameters.append(new CParameterWrapper(m_Device->parameter(i),this));
#ifdef Q_OS_IOS
    titleBar = new CustomTitleBar(parent,this,Device->deviceID(),FixedSize);
    connect(titleBar, &CustomTitleBar::closeRequested, this, &CSoftSynthsForm::close);
#endif
}

CSoftSynthsForm::~CSoftSynthsForm()
{
    QMutexLocker locker(&mutex);
    stopTimer();
    qDebug() << "~CSoftSynthsForm";
    qDeleteAll(parameters);
}

void CSoftSynthsForm::serialize(QDomLiteElement* xml) const
{
    QDomLiteElement* Position=xml->elementByTagCreate("Items")->elementByTagCreate("Position");
    Position->setAttribute("Top",pos().y(),0);
    Position->setAttribute("Left",pos().x(),0);
    if (!m_FixedSize) {
        Position->setAttribute("Height",geometry().height(),0);
        Position->setAttribute("Width",geometry().width(),0);
    }
    Position->setAttribute("Visible",isVisible(),false);
}

void CSoftSynthsForm::unserialize(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    if (!xml) return;
    if (const QDomLiteElement* Items=xml->elementByTag("Items")) {
        if (const QDomLiteElement* Position=Items->elementByTag("Position")) {
            if (!m_FixedSize) {
                QSize s(Position->attributeValueInt("Width"),Position->attributeValueInt("Height"));
#ifdef Q_OS_IOS
                QScreen *screen = QGuiApplication::primaryScreen();
                if (screen) {
                    QSize maxSize = screen->availableGeometry().size() * 0.9;
                    QSize newSize(qMin(maxSize.width(),s.width()),qMin(maxSize.height(),s.height()));
                    if (newSize != s) s = newSize;
                }
#endif
                resize(s);
            }
            const bool Visible=Position->attributeValueBool("Visible");
            if (Visible) if (m_FixedSize) show();
            setVisible(Visible);
            QPoint p(Position->attributeValueInt("Left"),Position->attributeValueInt("Top"));
#ifdef Q_OS_IOS
            p = QPoint(qMax(p.x(),10),qMax(p.y(),20));
#endif
            move(p);
#ifdef Q_OS_IOS
            placeTitlebar(Visible);
#endif
        }
        else
        {
            setVisible(false);
        }
    }
    else
    {
        setVisible(false);
    }
}

void CSoftSynthsForm::serializeCustom(QDomLiteElement* /*xml*/) const
{
}

void CSoftSynthsForm::unserializeCustom(const QDomLiteElement* /*XML*/)
{
}

void CSoftSynthsForm::stopTimer()
{
    if (m_TimerID) killTimer(m_TimerID);
    m_TimerID = 0;
}
#ifdef Q_OS_IOS
void CSoftSynthsForm::placeTitlebar(bool visible) {
    qDebug() << "CSoftSynthsForm placeTitlebar" << visible;
    if (visible) {
        titleBar->setGeometry(this->geometry().left(),this->geometry().top()-16,this->geometry().width(),16);
    }
    titleBar->setVisible(visible);
}
#endif
void CSoftSynthsForm::setVisible(bool visible)
{
    QMutexLocker locker(&mutex);
    setWindowTitle(m_Device->deviceID());
    qDebug() << "setVisible" << size() << pos();
    if (m_FixedSize) (size()==QSize(0,0)) ? setFixedSize(sizeHint()) : setFixedSize(size());
    QDialog::setVisible(visible);
    if (visible) fixMaximizeButton(this,!m_FixedSize);
    //if (visible) updateHost();
#ifdef Q_OS_IOS
    placeTitlebar(visible);
#endif
}

void CSoftSynthsForm::show()
{
    QMutexLocker locker(&mutex);
    setWindowTitle(m_Device->deviceID());
    qDebug() << "Show event" << size() << pos();
    if (m_FixedSize) (size()==QSize(0,0)) ? setFixedSize(sizeHint()) : setFixedSize(size());
    (pos()==QPoint(0,0)) ? QDialog::show() : setVisible(true);
    fixMaximizeButton(this,!m_FixedSize);
    //updateHost();
}

void CSoftSynthsForm::closeEvent(QCloseEvent *event)
{
    m_Device->hideForm();
    event->ignore();
}

bool CSoftSynthsForm::event(QEvent *event)
{
    //qDebug() << "CSoftSynthsForm::event" << event->type();
    bool ret = QDialog::event(event);
    if (event->type()==QEvent::NonClientAreaMouseButtonPress) {
        qDebug() << "CSoftSynthsForm non client click";
        m_Device->activate();
        CSoftSynthsForm::updateHost();
#ifdef Q_OS_IOS
        titleBar->setZOrder();
#endif
    }
#ifdef Q_OS_IOS
    else if (event->type() == QEvent::WindowActivate) {
        titleBar->setZOrder();
    }
#endif
    else if (event->type() == QEvent::ActivationChange) {
        if (this->isActiveWindow() && this->underMouse()) {
            m_Device->activate();
            CSoftSynthsForm::updateHost();
//#ifdef Q_OS_IOS
//            placeTitlebar(true);
//#endif
        }
        if (findChild<QMenuBar*>()) {
            qDebug() << this << parentWidget() << QApplication::activeWindow() << QApplication::activePopupWidget();
            qDebug() << "CSoftSynthsForm activate" << this->isActiveWindow() << parentWidget()->isActiveWindow();
            //const bool enabled = (!this->isActiveWindow() || parentWidget()->isActiveWindow()) && parentWidget()->isActiveWindow();
            const bool enabled = (qApp->activeWindow() == parentWidget());
            for (const QMenuBar* m : (const QList<QMenuBar*>)parentWidget()->findChildren<QMenuBar*>("",Qt::FindDirectChildrenOnly)) {
                for (QAction* a : (const QList<QAction*>)m->actions()) {
                    if (a->menu()) a->menu()->setEnabled(enabled);
                }
            }
            for (QToolBar* t : (const QList<QToolBar*>)parentWidget()->findChildren<QToolBar*>("",Qt::FindDirectChildrenOnly)) t->setEnabled(enabled);
        }
    }
    else if (event->type() == QEvent::Resize) {
        qDebug() << "CSoftSynthsForm resize";
        if (isVisible()) CSoftSynthsForm::updateHost();
#ifdef Q_OS_IOS
        qDebug() << "Resize titlebar";
        titleBar->setGeometry(this->geometry().left(),this->geometry().top()-16,this->geometry().width(),16);
#endif
    }
    else if (event->type() == QEvent::Show) {
        qDebug() << "CSoftSynthsForm show";
        CSoftSynthsForm::updateHost();
#ifdef Q_OS_IOS
        placeTitlebar(true);
#endif
    }
    return ret;
}

void CSoftSynthsForm::setParameter(int index, int value)
{
    QMutexLocker locker(&mutex);
    m_Device->parameter(index)->setValue(value);
}

void CSoftSynthsForm::setParameter(QString name, int value)
{
    QMutexLocker locker(&mutex);
    //m_Device->setParameterValue(name,value);
    for (int i=0;i<m_Device->parameterCount();i++)
    {
        if (m_Device->parameter(i)->Name==name)
        {
            m_Device->parameter(i)->setValue(value);
            return;
        }
    }
}

void CSoftSynthsForm::updateHost() {
    qDebug() << "CSoftSynthsForm updateHost";
    QMetaObject::invokeMethod(this,&CSoftSynthsForm::sendUpdateRequest,Qt::QueuedConnection);
    //sendUpdateRequest();
}

void CSoftSynthsForm::sendUpdateRequest() {
    qDebug() << "CSoftSynthsForm sendUpdateRequest";
    m_Device->updateHostParameter();
}
