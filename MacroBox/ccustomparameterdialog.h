#ifndef CCUSTOMPARAMETERDIALOG_H
#define CCUSTOMPARAMETERDIALOG_H

#include <QDialog>
#include "cdevicelist.h"
#include <QTreeWidget>
#include "cmacroboxform.h"

namespace Ui {
class CCustomParameterDialog;
}

class CCustomParameterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CCustomParameterDialog(QWidget *parent = nullptr);
    ~CCustomParameterDialog();
    void fill(CDeviceList* l, CCustomParameterList* p, IDevice* d);
private slots:
    void itemClicked(QTreeWidgetItem* i, int col);
    void selectParameter(QListWidgetItem* i);
    void editParameterName(QListWidgetItem* i);
    void editType(int t);
    void editUnit(QString s);
    void editMin(double v);
    void editMax(double v);
    void editList();
    void editDecimal(QString s);
    void addParameterClicked();
    void addParameter(QString id);
    void removeParameterClicked();
    void acceptDialog();
    void reorderParameters();
    void applyDialog();
private:
    Ui::CCustomParameterDialog *ui;
    CDeviceList* m_DeviceList;
    CCustomParameterList* m_CustomParameters;
    IDevice* m_ParentDevice;
    QDomLiteElement m_xml;
    QDomLiteElement* customParameterElement(const QString& customParameterName);
    QDomLiteElement* parameterElement(const QString& customParameterName, const QString& parameterID);
    QDomLiteElement* parameterElement(const QDomLiteElement* c, const QString& parameterID);
    QTreeWidgetItem* parameterItem(const QString& parameterID);
    QDomLiteElement* currentCustomParameter();
};

#endif // CCUSTOMPARAMETERDIALOG_H
