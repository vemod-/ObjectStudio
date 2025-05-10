#ifndef CSTEREOPEAK_H
#define CSTEREOPEAK_H

#include <QFrame>

namespace Ui {
    class CStereoPeak;
}

class CStereoPeak : public QFrame
{
    Q_OBJECT

public:
    explicit CStereoPeak(QWidget *parent = nullptr);
    ~CStereoPeak();
    void setValues(const float L, const float R);
    void reset();
    void setMargin(int margin);
    void setMax(int max);
private:
    Ui::CStereoPeak *ui;
};

#endif // CSTEREOPEAK_H
