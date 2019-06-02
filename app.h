#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include "imageprocessing.h"
#include <QAbstractButton>
#include <QLabel>
#include <QHBoxLayout>

namespace Ui {
class App;
}

class App : public QMainWindow
{
Q_OBJECT

public:
    explicit App(QWidget *parent = nullptr);
    ~App();

private slots:
    void on_btnOpen_clicked();

    void on_comboBoxEnhan_currentIndexChanged(int index);

    void on_btnSave_clicked();

    void on_sliderThres_valueChanged(int value);

    void on_btnApply_clicked();

    void on_btnUndo_clicked();

    void on_btnRedo_clicked();

    void on_dial_valueChanged(int value);

    void on_algo_currentIndexChanged(int index);

    void on_r1_valueChanged(int value);

    void on_s1_valueChanged(int value);

    void on_r2_valueChanged(int value);

    void on_s2_valueChanged(int value);

    void on_sliderA_valueChanged(int value);

    void on_sliderB_valueChanged(int value);

    void on_sliderVal_valueChanged(int value);

    void on_btnG1_buttonPressed(QAbstractButton *button);

    void on_comboBoxSeg_currentIndexChanged(int index);

    void on_comboBoxSegEdge_currentIndexChanged(int index);

    void on_comboBoxMor_currentIndexChanged(int index);

    void on_tabWidget_currentChanged(int index);

private:
    void showImage();
    void showHistogram();
    void showMessage();
    void showAll();

    void menuEnhan();
    void menuFil();
    void menuSeg();
    void menuMor();

private:
    Ui::App *ui;
    std::vector<Image> mImages;
    std::vector<Image>::iterator curImage;
};

#endif // APP_H
