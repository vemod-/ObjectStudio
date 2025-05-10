#ifndef CPRESETSFORM_H
#define CPRESETSFORM_H

#include <QDialog>
#include <QList>

namespace Ui {
class CPresetsForm;
}

class CPresetsForm : public QDialog
{
    Q_OBJECT

public:
    explicit CPresetsForm(QWidget *parent = 0);
    ~CPresetsForm();
    void fill(const QList<uint> sampleRates);
private:
    Ui::CPresetsForm *ui;
private slots:
    void apply();
    void selectFolder();
};

#endif // CPRESETSFORM_H
