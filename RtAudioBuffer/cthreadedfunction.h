#ifndef CTHREADEDFUNCTION_H
#define CTHREADEDFUNCTION_H

#include <QThread>
//#include <QVariant>
#include <QApplication>
#include <unistd.h>
#include <QProgressDialog>
//#include <QProgressBar>
//#include <QMetaMethod>
#include <QLabel>
#include <QMovie>
#include <QtConcurrent/QtConcurrent>
#include <QScreen>

class WorkerThread : public QThread
{
    Q_OBJECT
    void run() override {
        emit resultReady();
    }
signals:
    void resultReady();
};

class CThreadedFunction : public QObject
{
    Q_OBJECT
public:
    template <typename Func2>
    CThreadedFunction(const typename QtPrivate::FunctionPointer<Func2>::Object *receiver, Func2 slot, QString data, QString text = QString(), QObject* parent = nullptr) : QObject(parent)
    {
        t = new WorkerThread();
        connect(t,&WorkerThread::resultReady,this,&CThreadedFunction::work,Qt::DirectConnection);
        connect(t,&WorkerThread::finished,t,&WorkerThread::deleteLater);
        connect(this,&CThreadedFunction::doJob,receiver,slot,Qt::DirectConnection);
        QProgressDialog* p = nullptr;
        QLabel* l = nullptr;
        QMovie* m;
        if (text == "spinner")
        {
            m = new QMovie("/Users/thomasallin/Downloads/arrows_circle_tcm6-65331.gif");
            l = new QLabel(static_cast<QWidget*>(parent));
            l->setWindowModality(Qt::ApplicationModal);
            l->setWindowFlag(Qt::FramelessWindowHint);
            l->setAttribute(Qt::WA_TranslucentBackground);
            m->jumpToFrame(0);
            QSize movie_size = m->currentImage().size();
            m->setScaledSize(movie_size);
            l->setMovie(m);
            l->setFixedSize(movie_size);
            l->show();
            m->start();
            QApplication::processEvents((QEventLoop::ExcludeUserInputEvents));
        }
        else if (!text.isEmpty())
        {
            p = new QProgressDialog(text, QString(), 0, 0, static_cast<QWidget*>(parent));
            p->setWindowModality(Qt::ApplicationModal);
            p->setWindowFlag(Qt::FramelessWindowHint);
            p->show();
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        m_Data = data;
        t->start();
        while (t->isRunning())
        {
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            usleep(1000);
        }
        if (p)
        {
            p->hide();
            delete p;
        }
        if (l)
        {
            l->hide();
            delete l;
        }
    }
protected slots:
    void work()
    {
        emit doJob(m_Data);
    }
signals:
    void doJob(QString d);
private:
    QString m_Data;
    WorkerThread* t;
};

class CConcurrentDialog : public QObject
{
    Q_OBJECT
public:
    CConcurrentDialog(QFuture<void> f,const QString& text)
    {
        QProgressDialog* p = nullptr;
        QLabel* l = nullptr;
        if (text == "spinner")
        {
            l = new QLabel;
            showSpinner(l,true);
        }
        else if (!text.isEmpty())
        {
            p = new QProgressDialog(text, QString(), 0, 0);
            p->setWindowModality(Qt::ApplicationModal);
            p->setWindowFlag(Qt::FramelessWindowHint);
            p->show();
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        while (!f.isFinished())
        {
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            usleep(1000);
        }
        if (p)
        {
            p->hide();
            delete p;
        }
        if (l)
        {
            l->hide();
            delete l;
        }
    }
    template <typename T, typename Class, typename Param1, typename Arg1>
    static void run(Class *object, T (Class::*fn)(Param1), const Arg1 &arg1) {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        CConcurrentDialog(QtConcurrent::run(fn,object,arg1),"spinner");
#else
        CConcurrentDialog(QtConcurrent::run(object,fn,arg1),"spinner");
#endif
    }
    static void showSpinner(QLabel*& l,bool smoke = false, QRect r = QRect()) {
        //l->setAttribute(Qt::WA_UpdatesDisabled,true);
        QMovie* m = new QMovie(":/loading.gif");
        if (r == QRect()) l->setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        l->setScaledContents(false);
        if (!smoke) {
            l->setStyleSheet("background:transparent");
        }
        else {
            l->setStyleSheet("background-color:rgba(0,0,0,40)");
        }
        m->jumpToFrame(0);
        QSize movie_size = m->currentImage().size();
        m->setScaledSize(movie_size);
        l->setMovie(m);
        l->setAlignment(Qt::AlignCenter);
        //l->setFixedSize(movie_size);
        //l->showMaximized();
        qDebug() << r;
        if (r == QRect()) {
            QSize s = QGuiApplication::primaryScreen()->size();
            l->setFixedSize(s);
            //l->move((s.width()*0.5)-(l->width()*0.5),(s.height()*0.5)-(l->height()*0.5));
            l->setWindowFlag(Qt::WindowStaysOnTopHint);
        }
        else {
            l->setFixedSize(r.size());
            //l->move(r.topLeft());
            //l->setWindowFlag(Qt::WindowStaysOnTopHint);
            //l->raise();
        }
        l->setVisible(true);
        if (r == QRect()) l->setWindowModality(Qt::ApplicationModal);
        //l->showFullScreen();
        //QApplication::processEvents((QEventLoop::ExcludeUserInputEvents));
        //l->setAttribute(Qt::WA_UpdatesDisabled,false);
        l->repaint();
        m->start();
/*
        for (int i = 0; i < 5; i++)
        {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);
            usleep(10000);
        }
 */
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers,200);
        //QApplication::processEvents((QEventLoop::ExcludeUserInputEvents));
    }
};

class CSpinLabel : public QObject {
    Q_OBJECT
public:
    CSpinLabel(QWidget* parent = nullptr);
    ~CSpinLabel();
private:
    QLabel* l;
    void GeometryChanged();
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // CTHREADEDFUNCTION_H
