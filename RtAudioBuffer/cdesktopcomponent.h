#ifndef CDESKTOPCOMPONENT_H
#define CDESKTOPCOMPONENT_H

//#include <QGraphicsView>
#include <QMouseEvent>
#include "idevice.h"
#include "cdevicelist.h"
#include "qiphotorubberband.h"
#include <qsignalmenu.h>
#include <QPixmap>
#include "../../QGraphicsViewZoomer/qgraphicsviewzoomer.h"
#include "cprojectapp.h"
//#include "ceditmenu.h"

#define _DocumentPath QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/Object Studio/"

namespace DesktopComponent
{
    enum Layers
    {
        BackGroundLayer,
        ShadowLayer,
        DragShadowLayer,
        DeviceLayer,
        DragDeviceLayer
    };
}

class JackRect : public QRect
{
public:
    JackRect(IJack* j=nullptr) { jack=j; }
    IJack* jack;
};

class CJackContainer
{
public:
    virtual ~CJackContainer(){}
    QList<JackRect> jackRects;
    QRect geometry;
    int jackIndex(const QPoint& Pos) const
    {
        for (int i=0;i<jackRects.size();i++)
        {
            if (jackRects.at(i).translated(geometry.topLeft()).contains(Pos)) return i;
        }
        return -1;
    }
    inline bool contains(const QPoint& Pos) const { return geometry.contains(Pos); }
    QPoint jackPos(const int Index) const
    {
        return (Index >= jackRects.size()) ? QPoint() : jackRects.at(Index).topLeft()+QPoint(4,4)+geometry.topLeft();
    }
    inline int jackCount() const { return jackRects.size(); }
    inline IJack* jack(const int Index) const { return jackRects.at(Index).jack; }
    virtual QList<QGraphicsItem*> paint(QGraphicsScene* Scene);
};

class CDeviceComponent : public CJackContainer
{
private:
    IDevice* m_Device;
    bool m_Active;
    QString m_ClassName;
    QPixmap* m_px;
public:
    void getPic()
    {
        geometry.setSize(QSize(120,60));
        if (m_px) delete m_px;
        m_px=nullptr;
        if (m_Device->hasUI())
        {
            const QPixmap* px = m_Device->picture();
            if (px)
            {
                //qDebug() << px->size();
                m_px=new QPixmap(px->scaled((geometry.size()-QSize(2,2))*2,Qt::KeepAspectRatio,Qt::SmoothTransformation));
                m_px->setDevicePixelRatio(2);
                delete px;
            }
        }
    }
    CDeviceComponent() : m_Device(nullptr), m_Active(false), m_px(nullptr) {
        geometry.setTopLeft(QPoint(100,100));
    }
    CDeviceComponent(IDevice* Device, const QString& ClassName) : m_Device(nullptr), m_Active(false), m_px(nullptr)
    {
        geometry.setTopLeft(QPoint(100,100));
        init(Device,ClassName);
    }
    virtual ~CDeviceComponent() { if (m_px) delete m_px; }
    void init(IDevice* Device, const QString& ClassName)
    {
        m_ClassName=ClassName;
        m_Device=Device;
        for (int i=0;i<Device->jackCount();i++) jackRects.append(JackRect(Device->jack(i)));
        getPic();
    }
    inline IDevice* device() const { return m_Device; }
    const inline QString className() const { return m_ClassName; }
    void setSelected(const bool Active) {
        m_Active=Active;
        //if (Active) getPic();
    }
    bool inside(const QRect& r) { return r.contains(geometry); }
    QList<QGraphicsItem*> paint(QGraphicsScene* Scene);
};

class CJackBar : public CJackContainer
{
public:
    CJackBar() {}
    virtual ~CJackBar() {}
    IJack* addJack(IJack* J)
    {
        jackRects.append(JackRect(J));
        return J;
    }
    QList<QGraphicsItem*> paint(QGraphicsScene* Scene);
    static const int height=12;
};

namespace Ui {
class CDesktopComponent;
}

class CDesktopComponent : public QGraphicsView, public IFileDocument, public IHost//, public IEditDocument
{
    Q_OBJECT

public:
    explicit CDesktopComponent(QWidget *parent = nullptr);
    ~CDesktopComponent();
    void init(QWidget* mainWindow, QWidget* parent = nullptr);
    CDeviceComponent* addDeviceComponent(IDevice* Device, const QString& ClassName);
    CDeviceComponent* addDevice(const QString& ClassName);
    void serialize(QDomLiteElement* xml) const;
    void undoSerialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    void undoUnserialize(const QDomLiteElement* xml);
    void serializeDevice(IDevice* d, const QRect& geometry, QDomLiteElement* xml) const;
    void serializeConnection(CInJack* jack, QDomLiteElement* xml) const;
    QPair<QString,QString> unserializeDevice(const QDomLiteElement* xml, const QPoint& StartPoint=QPoint(), bool ReIndex=false);
    void unserializeConnection(const QDomLiteElement* xml, const QList<QPair<QString,QString>>& ReIndexer=QList<QPair<QString,QString>>());
    IJack* addJack(IJack* Jack, int PolyIndex);
    void clear();
    void hideForms();
    //IHost
    void parameterChange(IDevice* device, const CParameter* parameter = nullptr);
    void activate(IDevice* Device);
    void takeString(IDevice* Device, const int type, const QString& s);
    CDeviceList* deviceList() { return &DeviceList; }
    void setZoom(double zoom);
    CMainMenu* MainMenu;
    bool findSuffix(const QString& path, const QString& filter);
    bool initWithFile(const QString& path, QPoint pos);
public slots:
    void NewDoc();
    void OpenDoc(QString);
    void SaveDoc(QString path);
    void CloseDoc();
    void DeleteDoc();
    void CopyDoc(QDomLiteElement* xml);
    void PasteDoc(const QDomLiteElement* xml);
    void toggleUI();
    void RemoveConnections();
    void DrawConnections();
    void SelectDevice(IDevice* d);
    void changeZoom(const double zoom);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void scrollContentsBy(int dx, int dy);
    void resizeEvent(QResizeEvent *event);
signals:
    void parametersChanged(IDevice *Device);
    void controlChanged(IDevice* Device, const CParameter* Parameter);
    void playStopped();
    void MilliSecondsChanged();
    void deviceAdded(IDevice* Device);
    void deviceRemoved(IDevice* Device);
    void devicesCleared();
    void connectionsChanged();
    //void selectionChanged(bool);
    void zoomChanged(double zoomfactor);
    void requestSerializeAutomationXML(QDomLiteElement*) const;
    void requestUnserializeAutomationXML(const QDomLiteElement*);
private:
    Ui::CDesktopComponent *ui;
    CDeviceList DeviceList;
    QGraphicsScene Scene;
    QiPhotoRubberband* Rubberband;
    QGraphicsViewZoomer* zoomer;
    bool MouseDown;
    bool Marked;
    QList<IDevice*> MarkList;

    bool Dragging;
    IJack* DragJack;
    QPoint DragJackPos;
    QDomLiteElement* DragBackup = nullptr;

    QList<CDeviceComponent*> Devices;

    CJackBar JackBar1;
    CJackBar JackBar2;

    QSignalMenu* PluginsPopup;
    QMenu* MacrosPopup;

    inline CDeviceComponent* currentDeviceComponent() const { return Devices[m_DeviceIndex]; }
    inline IDevice* currentDevice() const { return currentDeviceComponent()->device(); }
    bool selectedDeviceIsValid() const;
    bool canCopy() const;
    void DisconnectJackBar(CJackBar& JackBar);
    int DeviceIndex(const QPoint& Pos) const;
    CDeviceComponent* addDevice(const QString& ClassName, const int ID);
    void RemoveDevice(IDevice* Device);
    void FillJackList();
    void hideRubberband();
    QList<QGraphicsItem*> DrawDeviceConnections(CDeviceComponent* Device,QList<CJackContainer*>& paintedContainers);
    Qt::CursorShape connectCursor(IJack* J1,IJack* J2);
    void SetConnectCursor(const QPoint& Pos);
    void ConnectDrop(const QPoint& Pos);
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

    QRecursiveMutex mutex;
    QWidget* m_MainWindow;
    QWidget* m_ParentWindow;
    QRect mapToScene(QRect r)
    {
        return QRectF(QGraphicsView::mapToScene(r.topLeft()),QGraphicsView::mapToScene(r.bottomRight())).toRect();
    }
    QRect mapFromScene(QRect r)
    {
        return QRectF(QGraphicsView::mapFromScene(r.topLeft()),QGraphicsView::mapFromScene(r.bottomRight())).toRect();
    }
private slots:
    void PluginMenuClicked(QString ClassName);
    void MacroMenuClicked(QString ProgramName);
};

#endif // CDESKTOPCOMPONENT_H
