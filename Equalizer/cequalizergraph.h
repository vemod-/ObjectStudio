#ifndef CEQUALIZERGRAPH_H
#define CEQUALIZERGRAPH_H

#include "qcanvas.h"

namespace Ui {
    class CEqualizerGraph;
}

class CEqualizerGraph : public QCanvas
{
    Q_OBJECT

public:
    explicit CEqualizerGraph(QWidget *parent = 0);
    ~CEqualizerGraph();
protected:
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent* e);
signals:
    void Changed();
private:
    Ui::CEqualizerGraph *ui;
};

#endif // CEQUALIZERGRAPH_H
