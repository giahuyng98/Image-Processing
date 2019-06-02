#ifndef MAT3X3_H
#define MAT3X3_H

#include <QFrame>
#include "imageprocessing.h"

namespace Ui {
class Mat3x3;
}

class Mat3x3 : public QFrame
{
    Q_OBJECT

public:
    explicit Mat3x3(QWidget *parent = nullptr);
    ~Mat3x3();
    void setMatrix(const Weight &weight);
    Weight getMatrix();
    Weight getMatrixAvg();

private:
    Ui::Mat3x3 *ui;
};

#endif // MAT3X3_H
