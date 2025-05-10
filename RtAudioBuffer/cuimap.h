#ifndef CUIMAP_H
#define CUIMAP_H

#include <QGraphicsView>
#include "idevice.h"

namespace Ui {
class CUIMap;
}

class CDeviceFrame
{
public:
    CDeviceFrame(const QPixmap* p, IDevice* d) {
        pixmap = p;
        device = d;
    }
    ~CDeviceFrame() { delete pixmap; }
    const QPixmap* pixmap;
    QRect rect;
    IDevice* device;
};

class CUIMap : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CUIMap(QWidget *parent = 0);
    ~CUIMap();
    void showMap(IDeviceParent* DeviceList,QWidget* parent,QWidget* replaces);
private slots:
    void drawDevices(QVariant itemSize);
signals:
    void deviceSelected();
protected:
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void resizeEvent(QResizeEvent* event);
private:
    Ui::CUIMap *ui;
    QGraphicsScene Scene;
    IDeviceParent* m_DL;
    int m_ColumnCount;
    int mouseOverDevice(QPoint Pos);
    QList<CDeviceFrame*> pixmaps;
    void loadDeviceImage(IDevice* d);
    void animate();
    void loadImages();
    int matrixTop;
    int matrixHeight;
    int deviceWidth;
};

#endif // CUIMAP_H
