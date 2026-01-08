#ifndef CCUSTOMJACKSDIALOG_H
#define CCUSTOMJACKSDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include "cmacroboxform.h"

namespace Ui {
class CCustomJacksDialog;
}

class CCustomJacksDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CCustomJacksDialog(QWidget *parent = nullptr);
    ~CCustomJacksDialog();

    void fill(QList<CDesktopComponent*>* desktops, CDesktopComponent* desktop, CCustomJackList *p, IDevice* d);
private:
    Ui::CCustomJacksDialog *ui;
    void selectCustomJack(QTreeWidgetItem *item);
    void itemSelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem*);
    void editJackName(QTreeWidgetItem *i);
    void reorderJacks();
    void addJackClicked();
    void addCustomJack(QString id);
    void removeJackClicked();
    void acceptDialog();
    void applyDialog();

    QTreeWidgetItem* addElementToList(const QDomLiteElement* xml);
    QDomLiteElement m_xml;
    QList<CDesktopComponent*>* m_Desktops;
    CDesktopComponent* m_Desktop;
    CCustomJackList* m_CustomJacks;
    IDevice* m_ParentDevice;
    //QList<IJack*>* JacksCreated;
    //QList<CInJack*>* InsideJacks;
    QDomLiteElement *customJackElement(const QString &customParameterName);
    QDomLiteElement *currentCustomJack();
    QTreeWidgetItem* inJacksItem();
    QTreeWidgetItem* outJacksItem();
};

#endif // CCUSTOMJACKSDIALOG_H
