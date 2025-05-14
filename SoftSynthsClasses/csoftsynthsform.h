#ifndef CSOFTSYNTHSFORM_H
#define CSOFTSYNTHSFORM_H

//#define MDI_INTERFACE
//#define TAB_INTERFACE
//#define EMBEDDED_INTERFACE
//#define GRAPHICWORKAREA_INTERFACE

#include <QDialog>
#include <QCloseEvent>
#include <QDomLite>
#include "cpresets.h"
#ifdef Q_OS_IOS
#include <QGesture>
#endif

#include "idevicebase.h"

void inline setFontSizeScr(QWidget* w, const double s) {
    QFont f(w->font()); f.setPointSizeF(s); w->setFont(f);
}

class IDevice;


class CParameterWrapper : public QObject, public IParameterHost
{
    Q_OBJECT
public:
    CParameterWrapper(CParameter* p, QObject* parent = nullptr);
    ~CParameterWrapper();
    void setOffset(int v);
    template <typename Func1, typename Func2>
    void connectToWidget(typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal = nullptr, Func2 slot = nullptr)
    {
        connectToWidget(sender,signal,sender,slot);
    }
    void updateParameter(const CParameter* p = nullptr);
public slots:
    void setValue(int v);
    void setIntValue(int v);
    void setPercentValue(float v);
    void setdBValue(float v);
    void setBoolValue(bool v);
signals:
    void intValueChanged(int v);
private:
    CParameter* m_OwnerParameter = nullptr;
    QObject* m_SignalControl = nullptr;
    QObject* m_SlotControl = nullptr;
    int m_ControlOffset = 0;
    QRecursiveMutex mutex;
    template <typename Func1, typename Func2>
    void connectToWidget(typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal = nullptr, typename QtPrivate::FunctionPointer<Func2>::Object *receiver = nullptr, Func2 slot = nullptr)
    {
        QMutexLocker locker(&mutex);
        m_SignalControl = reinterpret_cast<QObject*>(sender);
        m_SlotControl = reinterpret_cast<QObject*>(receiver);
        if (signal) connect(sender,signal,this,&CParameterWrapper::setIntValue);
        if (slot) connect(this,&CParameterWrapper::intValueChanged,receiver,slot);
    }
};

#ifdef Q_OS_IOS
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>

class customSizeCorner : public QDialog {
    Q_OBJECT

public:
    explicit customSizeCorner(QWidget *parent = nullptr, QWidget* form = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
private:
    QLabel* cornerLabel;
    QPoint clickPosition;
    QWidget* parentForm;
};

class CustomTitleBar : public QDialog {
    Q_OBJECT

public:
    explicit CustomTitleBar(QWidget *parent = nullptr, QWidget* form = nullptr, QString label = QString(), bool fixedSize = true);
    void setVisible(bool v) override;
    void setZOrder();
signals:
    void closeRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;  // Hantera gestures
private:
    bool gestureEvent(QGestureEvent *event);
    QLabel *titleLabel;
    QPushButton *closeButton;
    QPoint clickPosition;
    QWidget* parentForm;
    customSizeCorner* sizeCorner = nullptr;
};
#endif

class CSoftSynthsForm : public QDialog, protected IPresetRef
{
    Q_OBJECT
public:
    CSoftSynthsForm(IDevice* Device, const bool FixedSize, QWidget* parent=nullptr);
    virtual ~CSoftSynthsForm();
    virtual void serialize(QDomLiteElement* xml) const;
    virtual void unserialize(const QDomLiteElement* xml);
    virtual void serializeCustom(QDomLiteElement* xml) const;
    virtual void unserializeCustom(const QDomLiteElement* xml);
    void stopTimer();
    QList<CParameterWrapper*>parameters;
    void updateHost();
public slots:
    void show();
    void setVisible(bool visible);
    void setParameter(QString name, int value);
    void setParameter(const int index, int value);
protected:
    int m_TimerID;
    bool m_FixedSize;
    IDevice* m_Device;
    void closeEvent(QCloseEvent *event);
    bool event(QEvent *event);
    QRecursiveMutex mutex;
    void sendUpdateRequest();
#ifdef Q_OS_IOS
    CustomTitleBar *titleBar;
    void placeTitlebar(bool visible);
#endif
};

#endif // CSOFTSYNTHSFORM_H
