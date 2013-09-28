#ifndef CDESKTOPCOMPONENT_H
#define CDESKTOPCOMPONENT_H

#include <QGraphicsView>
#include <QMouseEvent>
#include "softsynthsclasses.h"
#include "cdevicelist.h"
#include "qiphotorubberband.h"
#include <qsignalmenu.h>

class JackRect : public QRect
{
public:
    IJack* Jack;
    int Index;
};

class CJackContainer
{
public:
    QList<JackRect> JackRects;
    QRect Geometry;
    const int InsideJack(const QPoint& Pos);
    const bool InsideMe(const QPoint& Pos);
    QPoint JackPos(const int Index);
    const int JackCount();
    IJack* GetJack(const int Index);
    virtual QList<QGraphicsItem*> Paint(QGraphicsScene* Scene);
};

class CDeviceComponent : public CJackContainer
{
private:
    IDevice* m_Device;
    bool m_Active;
    QString m_ClassName;
public:
    CDeviceComponent();
    ~CDeviceComponent();
    QString Name;
    void Init(IDevice* Device, const QString& ClassName);
    IDevice* Device(void);
    QString ClassName(void);
    void Select(const bool Active);
    QList<QGraphicsItem*> Paint(QGraphicsScene* Scene);
};

class CJackBar : public CJackContainer
{
public:
    CJackBar();
    ~CJackBar();
    IJack* AddJack(IJack* J);
    QList<QGraphicsItem*> Paint(QGraphicsScene* Scene);
};

namespace Ui {
class CDesktopComponent;
}

class CDesktopComponent : public QGraphicsView, public IHost
{
    Q_OBJECT

public:
    explicit CDesktopComponent(QWidget *parent = 0);
    ~CDesktopComponent();

    CDeviceList DeviceList;
    void AddDevice(IDevice* Device, const QString& ClassName);
    const bool AddDevice(const QString& ClassName, void* MainWindow);
    const QString Save(const QString& Mode);
    void Load(const QString& XML);
    IJack* CreateInsideJack(int ProcIndex, IJack* ConnectTo, IDeviceBase* DeviceClass);
    IJack* AddJack(IJack* Jack, int PolyIndex);
    void SaveFile(const QString& Path=QString());
    void Play(const bool FromStart);
    void Pause(void);
    void SetPoly(const int Count);
    void UpdatePoly(void);
    void Tick(void);
    void Clear(void);
    QWidget* MainWindow;
    void ParameterChange();
    void Activate(IDevice* Device);
    void SetFileMenu(QMenu* Menu);
    void HideForms();
public slots:
    void New();
    void Open();
    void Save();
    void SaveAs();
    void Cut();
    void Copy();
    void Paste();
    void Execute();
    void CopyParameters();
    void PasteParameters();
    void RemoveConnections();
    void ParameterPopup(QPoint Pos);
    void ChangeParameter(IDevice* Device, int ParameterIndex, int Value);
    void OpenFile(const QString& Path);
    void SavePresetAs();
    void RemovePreset(QString PresetName);
    void OpenPreset(QString PresetName);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void scrollContentsBy(int dx, int dy);
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);
    void showEvent(QShowEvent *event);
signals:
    void UpdateParameters(IDevice *Device, QString Title);
    void StopPlaying();
private:
    Ui::CDesktopComponent *ui;
    QGraphicsScene Scene;
    QiPhotoRubberband* Rubberband;
    bool MouseDown;
    bool Marked;
    QList<IDevice*> MarkList;

    bool Dragging;
    IJack* DragJack;
    QPoint DragJackPos;
    QList<CDeviceComponent> Devices;
    QList<CDeviceList> PolyDevices;

    CJackBar JackBar1;
    CJackBar JackBar2;

    QSignalMenu* PluginsPopup;

    void DisconnectJackBar(CJackBar& JackBar);
    const int DeviceIndex(const QPoint& Pos);
    const bool AddDevice(const QString& ClassName, const int ID, void* MainWindow);
    void RemoveDevice(IDevice* Device);
    void DisconnectDevice(IDevice* Device);
    void FillJackList(void);
    QList<QGraphicsItem*> DrawArrow(const QPoint& OutPoint, const QPoint& InPoint, QColor Color);
    void DrawConnections();
    QList<QGraphicsItem*> DrawDeviceConnections(CDeviceComponent& Device,QList<CJackContainer*>& paintedContainers);
    void ShowParameters(IDevice* Device);
    void LoadParameters(QDomLiteElement* Device,IDevice* D);
    Qt::CursorShape CanConnect(IJack* J1,IJack* J2);
    void Connect(IJack* J1,IJack* J2);
    void Connect(const QString& J1,const QString& J2);
    void Disconnect(const QString& J1,const QString& J2);
    void SetConnectCursor(const QPoint& Pos);
    void ConnectDrop(const QPoint& Pos);
    const bool DeviceInsideRect(CDeviceComponent& D);
    IJack* MouseOverJack(const QPoint& Pos, QPoint& JackPoint);
    IJack* MouseOverJack(const QPoint &Pos);
    QList<QGraphicsItem*> DragList;
    void SelectDevice(const int Index);
    int m_DeviceIndex;
    bool m_MD;
    QPoint Start;
    QPoint StartPos;
    QPoint StartPoint;
    QPoint MousePos;
    QRect CopyRect;
    QSignalMenu* JackPopup;
    QString MenuJackID;
    QString FileName;
    QMenu* ParametersMenu;
    QSignalMenu* ParameterPresetsMenu;
    QMenu* DesktopMenu;
    QMenu* DeviceMenu;
    QMenu* MarkMenu;
    QSignalMenu* RecentMenu;
    QSignalMenu* RecentPopup;
    QAction* actionPaste;
    QAction* actionPasteParameters;

    void AddRecentFile(const QString& Path);
    void CreateRecentMenu(QSignalMenu* m);
private slots:
    void ToggleConnection(QString JackID);
    void PluginMenuClicked(QString ClassName);
};

#endif // CDESKTOPCOMPONENT_H
