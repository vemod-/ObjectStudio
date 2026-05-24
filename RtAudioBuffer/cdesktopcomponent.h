#ifndef CDESKTOPCOMPONENT_H
#define CDESKTOPCOMPONENT_H

//#include <QGraphicsView>
#include <QMouseEvent>
#include "idevice.h"
#include "cdevicelist.h"
//#include "qiphotorubberband.h"
#include <QtWidgets/qlineedit.h>
#include <QGraphicsProxyWidget>
#include <qsignalmenu.h>
#include <QPixmap>
#include "../../QGraphicsViewZoomer/qgraphicsviewzoomer.h"
#include "cprojectapp.h"
#include "qdprpixmap.h"
#include "qgraphicsitemlist.h"
//#include "ceditmenu.h"

#ifdef Q_OS_IOS
#define _DocumentPath QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/"
#else
#define _DocumentPath QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/Object Studio/"
#endif

#define deviceResolution 80
#define deviceTopSize QSize(deviceResolution * 3,deviceResolution * 1.5)
#define deviceBackSize QSize(deviceResolution * 3,48)
#define jackSize QSize(10,10)
#define jackPen 2

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
    JackRect(IJack* j) {
        jack = j;
        setSize(jackSize);
    }
    IJack* jack;
    bool match(const JackRect& r) {
        if (r.jack != jack) return false;
        if (r.topLeft() != topLeft()) return false;
        return true;
    }
};

class CJackContainer
{
public:
    virtual ~CJackContainer(){
        ContainerItem.removeOne(&jackItems);
    }
    QList<JackRect> jackRects;
    QGraphicsContainerItem jackItems;
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
        return (Index >= jackRects.size()) ? QPoint() : jackRects.at(Index).center() + geometry.topLeft();
    }
    inline int jackCount() const { return jackRects.size(); }
    inline IJack* jack(const int Index) const { return jackRects.at(Index).jack; }
    virtual void paint(QGraphicsScene* Scene);
    QGraphicsContainerItem ContainerItem;
private:
    QList<JackRect> paintedJacks;
    QList<QGraphicsPixmapItem*> plugItems;
};

class CDeviceComponent : public CJackContainer
{
public:
    enum DeviceView {
        TopView,
        BackView,
        FrontView,
        Drawing
    };
private:
    IDevice* m_Device;
    bool m_Active;
    QString m_ClassName;
    DeviceView m_View = Drawing;
    QDPRPixmap m_frontPix;
    QDPRPixmap createDrawing(bool active);
    QGraphicsProxyWidget* m_DeviceLabel;
    QGraphicsPixmapItem* m_DeviceUIPic;
    QGraphicsPixmapItem* m_DevicePic;
    QGraphicsPixmapItem* m_DeviceShadowPic;
    int m_OldView = -1;
    bool m_OldActive = false;
    bool m_UIPic = false;
    double m_InJackFactor = 0.1;
    double m_OutJackFactor = 0.1;
public:
    void getPic();
    CDeviceComponent();
    CDeviceComponent(IDevice* Device, const QString& ClassName);
    virtual ~CDeviceComponent();
    void init(IDevice* Device, const QString& ClassName);
    IDevice* device() const;
    const QString className() const;
    void setSelected(const bool Active);
    bool inside(const QRect& r);
    DeviceView view();
    void setView(DeviceView v);
    QRect captionRect;
    void paint(QGraphicsScene* Scene);
    void setFrontPix(const QDPRPixmap& p);
    bool frontPixSet();
    QGraphicsToolButton* rotateButton;
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
    void paint(QGraphicsScene* Scene);
    static const int height = 12;
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
    void addInsideJack(IJack* J, IDevice* d, const QString& alias = QString());
    void removeJack(IJack* jack, int PolyIndex);
    void reorderJackbarJacks(QList<IJack*>* jacksCreated);
    //void removeDeviceJack(IJack* jack);
    //void addDeviceJack(IJack* jack);
    void updateDeviceJacks();
    void clear();
    void clearJacksCreated();
    void hideForms();
    //IHost
    void parameterChange(IDevice* device, const CParameter* parameter = nullptr);
    void closeAutomation(IDevice* /*Device*/);
    void activate(IDevice* Device);
    void takeString(IDevice* Device, const int type, const QString& s);
    CDeviceList* deviceList() { return &DeviceList; }
    void setZoom(double zoom);
    CMainMenu* MainMenu;
    bool findSuffix(const QString& path, const QString& filter);
    bool initWithFile(const QString& path, QPoint pos);
    QList<CInJack*> InsideJacks;
    QList<IJack*> JacksCreated;
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
    void RemoveDevice(IDevice* Device);
    void moveDevice(int deviceIndex, int move);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void scrollContentsBy(int dx, int dy);
    void resizeEvent(QResizeEvent *event);
    bool event(QEvent* event);
signals:
    void parametersChanged(IDevice *Device);
    void controlChanged(IDevice* Device, const CParameter* Parameter);
    void playStopped();
    void MilliSecondsChanged();
    void deviceAdded(IDevice* Device);
    void deviceRemoved(IDevice* Device);
    void devicesReordered(int deviceIndex, int move);
    void devicesCleared();
    void connectionsChanged();
    void jacksChanged();
    //void selectionChanged(bool);
    void zoomChanged(double zoomfactor);
    void requestSerializeAutomationXML(QDomLiteElement*) const;
    void requestUnserializeAutomationXML(const QDomLiteElement*);
    void requestCloseAutomation(IDevice* device);
    void requestParametersPixmap(IDevice* device, QPixmap* p);
private:
    Ui::CDesktopComponent *ui;
    CDeviceList DeviceList;
    QGraphicsScene Scene;
    //QiPhotoRubberband* Rubberband;
    QGraphicsIPhotoRubberband* selectRect;
    QGraphicsViewZoomer* zoomer;
    bool MouseDown;
    bool Marked;
    QList<IDevice*> MarkList;
    QGraphicsProxyWidget* m_LineEdit = nullptr;

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
    void FillJackList();
    void hideRubberband();
    QGraphicsItemList DrawDeviceConnections(CDeviceComponent* Device, QList<CJackContainer*>& paintedContainers);
    void ConnectDrop(const QPoint& Pos);
    IJack* MouseOverJack(const QPoint& Pos, QPoint& JackPoint);
    IJack* MouseOverJack(const QPoint &Pos);
    int MouseOverRotateButton(const QPoint& Pos);
    QGraphicsItemList DragList;
    QGraphicsItemList ConnectionsList;
    void SelectDevice(const int Index);
    void RemoveDeviceNoPaint(IDevice* Device);
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
    void editDeviceCaption();
};

#endif // CDESKTOPCOMPONENT_H
