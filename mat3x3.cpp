#include "mat3x3.h"
#include "ui_mat3x3.h"

Mat3x3::Mat3x3(QWidget *parent) :
    QFrame(parent),
ui(new Ui::Mat3x3)
{
    ui->setupUi(this);
}

Mat3x3::~Mat3x3()
{
    delete ui;
}

void Mat3x3::setMatrix(const Weight &weight)
{
    ui->l00->setText(QString::number(weight[0]));
    ui->l01->setText(QString::number(weight[1]));
    ui->l02->setText(QString::number(weight[2]));
    ui->l10->setText(QString::number(weight[3]));
    ui->l11->setText(QString::number(weight[4]));
    ui->l12->setText(QString::number(weight[5]));
    ui->l20->setText(QString::number(weight[6]));
    ui->l21->setText(QString::number(weight[7]));
    ui->l22->setText(QString::number(weight[8]));
}

Weight Mat3x3::getMatrix()
{
    return {
        ui->l00->text().toFloat(),
        ui->l01->text().toFloat(),
        ui->l02->text().toFloat(),
        ui->l10->text().toFloat(),
        ui->l11->text().toFloat(),
        ui->l12->text().toFloat(),
        ui->l20->text().toFloat(),
        ui->l21->text().toFloat(),
        ui->l22->text().toFloat()};
}

Weight Mat3x3::getMatrixAvg()
{
    Weight result = getMatrix();
    float sum = std::accumulate(result.begin(), result.end(), 0);
    for(float &x: result){
        x /= sum;
    }
    return result;
}
