#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include <QApplication>
#include <QMainWindow>
#include "corebuffer.h"
//#include <softsynthsclasses.h>
//#include <QEvent>
//#include <QtDebug>
#include <QToolButton>
#include "ctimelineslider.h"
#include "cprojectapp.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public IMainPlayer
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void skip(const ulong64 samples);
    void play(const bool FromStart);
    void pause();
    bool isPlaying() const;
    ulong ticks() const;
    ulong milliSeconds() const;
    ulong64 samples() const;
    ulong64 currentSample() const;
    ulong currentMilliSecond() const;
    void renderWaveFile(const QString path);
public slots:
    void Play();
    void Continue();
    void Skip(const unsigned long long samples);
    void Stop();
    void Record();
    void StopRecording();
    void TogglePlay(bool value);
    void ToggleRecord(bool value);
    void SetVolume(int vol);
    void InDriverChange(QString inDriverName);
    void OutDriverChange(QString outDriverName);
    void Clear();
    void Preferences();
    void cascadeUIs();
    void exportWav();
    bool dropfile(const QString& path, QPoint pos = QPoint());
protected:
    void timerEvent(QTimerEvent *event);
    void closeEvent(QCloseEvent* event);
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);
    /*
    bool event(QEvent* event) {
        if ( event->type() == QEvent::WindowActivate )
        {
            qDebug() << "ApplicationActivate";
            if (!menuBar()->isVisible()) {
            //if (QDialog* d = findChild<QDialog*>()) {
            //    qDebug() << d->windowTitle();
            //    if (QMenuBar* m = d->findChild<QMenuBar*>()) {
            //        qDebug() << m->children().count();
                    //m->setNativeMenuBar(false);
                    //menuBar()->setNativeMenuBar(true);
                    for (QAction* a : ActiveList)
                    {
                        menuBar()->addAction(a);
                        a->setEnabled(true);
                    }
                    menuBar()->setVisible(true);
                    //for (QAction* a : menuBar()->actions()) a->setVisible(true);
            //    }
            }
        }
        else if ( event->type() == QEvent::WindowDeactivate )
        {
            qDebug() << "ApplicationDeactivate";
            if (QWidget* w = qApp->activeWindow()) {
                if (QMenuBar* m = w->findChild<QMenuBar*>()) {
                    qDebug() << m->actions().count();
                    menuBar()->setVisible(false);
                    //menuBar()->setNativeMenuBar(false);
                    //m->setNativeMenuBar(true);
                    //QList<QAction*> l = menuBar()->actions();
                    ActiveList.clear();
                    for (QAction* a : menuBar()->actions())
                    {
                        if (a->menu()) {
                            ActiveList.append(a);
                            menuBar()->removeAction(a);
                        }
                    }
                    //for (QAction* a : l) a->setVisible(false);
                }
            }
            //if (findChild<QDialog*>()) qDebug() << findChild<QDialog*>()->windowTitle();
        }
        return QMainWindow::event(event);
    }
*/
private:
    void setButton(QToolButton* b,const bool checked,const QString& icon);
    int m_TimerID;
    Ui::MainWindow *ui;
    CCoreMainBuffers MainBuffers;
    QRecursiveMutex mutex;
    CTimeLineSlider* m_TimeLineSlider;
    bool projectView = true;
    QMenu* menuFile;
    QMenu* menuView;
    QMenu* menuEdit;
    QAction* actionExportWav;
    QAction* actionPreferences;
    QAction* actionDrivers;
    QAction* actionUImap;
    QAction* actionHideUIs;
    QAction* actionCascadeUIs;
    CProjectApp* m_ProjectApp;
    QList<QAction*> ActiveList;
};

class CObjectStudioApplication : public QApplication
{
    Q_OBJECT

public:
    MainWindow w;
    CObjectStudioApplication( int argc, char * argv[] ) : QApplication( argc, argv ) {}
    /*
    bool event( QEvent * pEvent )
    {
        if ( pEvent->type() == QEvent::ApplicationActivate )
        {
            w.activateWindow();
            qDebug() << "ApplicationActivate";
        }
        else if ( pEvent->type() == QEvent::ApplicationDeactivate )
        {
            qDebug() << "ApplicationDeactivate";
        }
        return QApplication::event( pEvent );
    }
    */
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            //qDebug() << "Open file" << openEvent->file();
            w.dropfile(openEvent->file());
        }
        return QApplication::event(event);
    }
};

#endif // MAINWINDOW_H
