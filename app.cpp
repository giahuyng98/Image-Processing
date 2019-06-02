#include "app.h"
#include "ui_app.h"
#include <QDebug>

static const Weight normal = {1, 1, 1, 1, 1, 1, 1, 1, 1};
static const Weight wavg = {1, 2, 1, 2, 4, 2, 1, 2, 1};
static const Weight lapf = {0, -1, 0, -1, 5, -1, 0, -1, 0};
static const Weight lapff = {0, 1, 0, 1, -4, 1, 0, 1, 0};
static const Weight zero = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static const Weight solf1 = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
static const Weight solf2 = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
static const Weight prew1 = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
static const Weight prew2 = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
static const Weight rob1 = {-1, 0, 0, 0, 1, 0, 0, 0, 0};
static const Weight rob2 = {0, -1, 0, 1, 0, 0, 0, 0, 0};
static const Weight point = {-1, -1, -1, -1, 8, -1, -1, -1, -1};

static const Weight hor = {-1, -1, -1, 2, 2, 2, -1, -1, -1};
static const Weight ver = {-1, 2, -1, -1, 2, -1, -1, 2, -1};
static const Weight p45 = {-1, -1, 2, -1, 2, -1, 2, -1, -1};
static const Weight n45 = {2, -1, -1, -1, 2, -1, -1, -1, 2};
static const Weight ste = {0, 1, 0, 1, 1, 1, 0, 1, 0};


enum {SOBEL_SEG_EDGE, ROBERT_SEG_EDGE, PREWITT_SEG_EDGE, LAPLACIAN_SEG_EDGE};

App::App(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::App)
{
    ui->setupUi(this);
    //    mImages.push_back(cv::imread("a.jpg", cv::IMREAD_GRAYSCALE));
    //    curImage = mImages.begin();
    //    ui->lbImage->setPixmap(QPixmap::fromImage(QImage(curImage->data, curImage->cols, curImage->rows, curImage->step1(),
    //             QImage::Format_Grayscale8)));
    //    showHistogram();

    ui->btnUndo->setDisabled(true);
    ui->btnRedo->setDisabled(true);
    ui->matrixSegPoint->setMatrix(point);
    ui->matrixSegEdgeHor->setMatrix(solf1);
    ui->matrixSegEdgeVer->setMatrix(solf2);
    ui->matrixFil->setMatrix(normal);
    ui->matrixSegLine->setMatrix(hor);
    ui->matrixMor->setMatrix(ste);
    connect(ui->btnG1, SIGNAL(buttonPressed(QAbstractButton*)), this, SLOT(on_btnG1_buttonPressed(QAbstractButton*)));
}

App::~App()
{
    delete ui;
}

void App::on_btnOpen_clicked()
{
    mImages.clear();
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Image"), "", tr("Images (*.png *.xpm *.jpg *.bmp)"));
    if (fileName.isNull() || fileName.isEmpty()){
        return;
    }

    Image image = cv::imread(fileName.toStdString(), cv::IMREAD_GRAYSCALE);
    if (image.empty()){
        return;
    }

    mImages.push_back(image);
    curImage = mImages.begin();
    showAll();
}

void App::on_btnSave_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
           tr("Save Image"), "", tr("Images (*.png *.xpm *.jpg *.bmp)"));
    if (fileName.isNull() || fileName.isEmpty()){
        return;
    }

    cv::imwrite(fileName.toStdString(), *curImage);
}

void App::showImage()
{
    if (mImages.empty()) return;
    QPixmap imageToShow = QPixmap::fromImage(
        QImage(curImage->data, curImage->cols, curImage->rows, curImage->step1(), QImage::Format_Grayscale8));
    if (ui->cbScaled->isChecked()){

        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        imageToShow = imageToShow.scaled(ui->scrollArea->viewport()->width(),
                           ui->scrollArea->viewport()->height(), Qt::KeepAspectRatio);
    } else {
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    }    
    ui->lbImage->setPixmap(imageToShow);
}

void App::showHistogram()
{
    QChart *chart = new QChart;
    QBarSet *barSet = new QBarSet("");
    QBarSeries *series = new QBarSeries;

    int maxCnt = 0;
    for(int grLvCnt : calcHistogram(*curImage)){
        maxCnt = std::max(maxCnt, grLvCnt);
        barSet->append(grLvCnt);
    }
    barSet->setBorderColor(Qt::gray);
    barSet->setBrush(QBrush(Qt::gray));
    series->append(barSet);
    series->setBarWidth(1.8);
    chart->addSeries(series);
    chart->setTitle("Histogram");

    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(0, DM - 1);
    axisX->setLabelFormat(tr("%d"));
    axisX->setMinorTickCount(2);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxCnt);
    axisY->setLabelFormat(tr("%d"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->hide();
    chart->setTheme(QChart::ChartTheme::ChartThemeLight);
    chart->setAnimationOptions(QChart::AnimationOption::NoAnimation);
    chart->setBackgroundPen(QPen(Qt::gray));

    ui->histogramChart->setRenderHint(QPainter::Antialiasing);
    ui->histogramChart->setChart(chart);
}

void App::showAll()
{
    ui->btnUndo->setDisabled(curImage == mImages.begin());
    ui->btnRedo->setDisabled(curImage == mImages.end() - 1);
    ui->statusBar->showMessage(tr("Size: ") + QString::number(curImage->cols)
                               + tr("x") + QString::number(curImage->rows)
                               + tr(" | ") + tr("Score: ") + QString::number(getScore(*curImage)));
    showImage();
    showHistogram();
}

void App::menuEnhan()
{
    enum {HISTOGRAM_ENHAN, NEGATIVE_ENHAN, THRESHOING_ENHAN,
           LOGARITHM_ENHAN, POWER_ENHAN, PICEWISE_ENHAN, GRAY_ENHAN, BIT_ENHAN};
    static const std::function<uchar(uchar, int)> bitFuncs[] = {viewBit, onBit, offBit};
    switch (ui->comboBoxEnhan->currentIndex()) {
    case HISTOGRAM_ENHAN:
        mImages.push_back(histogramTransforms(*curImage));
        break;
    case NEGATIVE_ENHAN:
        //            mImages.push_back(255 - *curImage);
        mImages.push_back(negativeTransforms(*curImage));
        break;
    case THRESHOING_ENHAN:
        mImages.push_back(thresholdingTransforms(*curImage, ui->sliderThres->value()));
        break;
    case LOGARITHM_ENHAN:
        mImages.push_back(logaricTransforms(*curImage, ui->lELog->text().toFloat()));
        break;
    case POWER_ENHAN:
        mImages.push_back(powerlawTransforms(*curImage, ui->lEPowerC->text().toFloat(),
                                             ui->lEPowerY->text().toFloat()));
        break;
    case PICEWISE_ENHAN:
        mImages.push_back(piecewiseLinearTransforms(
            *curImage, ui->r1->value(), ui->s1->value(), ui->r2->value(), ui->s2->value()));
        break;
    case GRAY_ENHAN:
        mImages.push_back(grayLevelSlicingTransforms(
            *curImage, ui->sliderA->value(), ui->sliderB->value(), ui->sliderVal->value()));
        break;
    case BIT_ENHAN:
        mImages.push_back(bitPlaneSlicingTransforms(
            *curImage, ui->dial->value(), bitFuncs[ui->comboBoxBit->currentIndex()]));
        break;
    }
}

void App::menuFil()
{
    static const Neighbor neighBor[] = {neighBorTransformsOmit, neighBorTransformsPad,
                                        neighBorTransformsRep, neighBorTransformsTrunc, neighBorTransformsAllow};
    static const Operator op[] = {min, max, med, sum, sum, sum, sum, sum, sum};
    enum {MIN, MAX, MED, AVG, WAVG, LAP, SOL1, SOL2, CUS};
    mImages.push_back(neighBor[ui->edge->currentIndex()](*curImage,
        op[ui->algo->currentIndex()], (ui->algo->currentIndex() == AVG || ui->algo->currentIndex() == WAVG) ?
        ui->matrixFil->getMatrixAvg() : ui->matrixFil->getMatrix()));

}

void App::menuSeg()
{
    enum {POINT, LINE, EDGE, THRES};
    static const Weight w1[] = {solf1, rob1, prew1};
    static const Weight w2[] = {solf2, rob2, prew2};
    switch (ui->comboBoxSeg->currentIndex()) {
    case POINT:
        mImages.push_back(neighBorTransformsOmit(*curImage, sum, point));
        break;
    case LINE:
        mImages.push_back(neighBorTransformsOmit(*curImage, sum, ui->matrixSegLine->getMatrix()));
        break;
    case EDGE:
    {
        int index = ui->comboBoxSegEdge->currentIndex();
        switch(index){
        case SOBEL_SEG_EDGE:
        case ROBERT_SEG_EDGE:        
        case PREWITT_SEG_EDGE:
            if (ui->checkBoxHor->isChecked() && ui->checkBoxVer->isChecked()){
                Image imHor = neighBorTransformsOmit(*curImage, sum, w1[index]);
                Image imVer = neighBorTransformsOmit(*curImage, sum, w2[index]);
//                mImages.push_back(powerlawTransforms(powerlawTransforms(imHor, 1, 2)
//                                                         + powerlawTransforms(imVer, 1, 2), 1, 0.5));
                mImages.push_back(imHor + imVer);
            } else if (ui->checkBoxHor->isChecked()){
                mImages.push_back(neighBorTransformsOmit(*curImage, sum, w1[index]));
            } else if (ui->checkBoxVer->isChecked()){
                mImages.push_back(neighBorTransformsOmit(*curImage, sum, w2[index]));
            } else {
                return;
            }
            break;
        case LAPLACIAN_SEG_EDGE:
            if (ui->checkBoxHor->isChecked()){
                mImages.push_back(neighBorTransformsOmit(*curImage, sum, lapff));
            } else {
                return;
            }
            break;       
        }
        break;
    }
    case THRES:
        uchar t = getT(*curImage, ui->lET->text().toFloat());
        ui->lbT->setText("Last found T = " + QString::number(t));
        mImages.push_back(thresholdingTransforms(*curImage, t));
        break;
    }

}

void App::menuMor()
{
    enum {ERO, DIL, OPEN, CLOSE, BOUND, FILL, FLOOD};
    switch(ui->comboBoxMor->currentIndex()){
    case ERO:
        mImages.push_back(morphology(*curImage, doErosion, ui->matrixMor->getMatrix()));
        break;
    case DIL:
        mImages.push_back(morphology(*curImage, doDilation, ui->matrixMor->getMatrix()));
        break;
    case OPEN:
        mImages.push_back(::open(*curImage, ui->matrixMor->getMatrix()));
        break;
    case CLOSE:
        mImages.push_back(::close(*curImage, ui->matrixMor->getMatrix()));
        break;
    case BOUND:
        mImages.push_back(::bound(*curImage, ui->matrixMor->getMatrix()));
        break;
    case FILL:
        if (ui->cbScaled->isChecked()){
            QMessageBox(QMessageBox::Icon::Critical,
                        tr("Error"), tr("Please disable scale option to use this feature!"),
                        QMessageBox::StandardButton::Ok).exec();
        } else {
            int x = ui->lbImage->getX();
            int y = ui->lbImage->getY();            
            mImages.push_back(::fill(*curImage, ui->matrixMor->getMatrix(), x, y));
        }
        break;
    case FLOOD:
        if (ui->cbScaled->isChecked()){
            QMessageBox(QMessageBox::Icon::Critical,
                        tr("Error"), tr("Please disable scale option to use this feature!"),
                        QMessageBox::StandardButton::Ok, this).exec();
        } else {
            mImages.push_back(floodFill(*curImage, ui->lbImage->getX(), ui->lbImage->getY()));
        }
        break;
    }

}

void App::on_sliderThres_valueChanged(int value)
{
    ui->lbThres->setText(tr("Threshold: ") + QString::number(value));
}

void App::on_btnApply_clicked()
{
    if (mImages.empty()){
        ui->statusBar->showMessage("Error, there is no image.");
        return;
    }
    enum {ENHAN, FILTER, SEGMENT, MORPHOLOGY, OPTION};
    mImages.erase(curImage + 1, mImages.end());

    switch(ui->tabWidget->currentIndex()){
    case ENHAN:
        menuEnhan(); break;
    case FILTER:
        menuFil(); break;

    case SEGMENT:
        menuSeg(); break;

    case MORPHOLOGY:
        menuMor(); break;

    case OPTION:
        showImage(); break;

    }
    curImage = mImages.end() - 1;
    showAll();
}

void App::on_comboBoxEnhan_currentIndexChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void App::on_btnUndo_clicked()
{
    if (curImage != mImages.begin()) --curImage;
    showAll();
}

void App::on_btnRedo_clicked()
{
    if (curImage != mImages.end() - 1) ++curImage;
    showAll();
}

void App::on_dial_valueChanged(int value)
{
    ui->lcdNumber->display(value);
}

void App::on_algo_currentIndexChanged(int index)
{
    enum {MIN, MAX, MED, AVG, WAVG, LAP, SOL1, SOL2, CUS};
    static const Weight look[] = {normal, normal, normal, normal,
                                  wavg, lapf, solf1, solf2, normal};
    ui->matrixFil->setDisabled(index != WAVG && index != CUS);
    ui->matrixFil->setMatrix(look[index]);
}

void App::on_r1_valueChanged(int value)
{
    ui->r1Label->setText(tr("r1 (") + QString::number(value) + tr(" /255)"));
}

void App::on_s1_valueChanged(int value)
{

    ui->s1Label->setText(tr("s1 (") + QString::number(value) + tr(" /255)"));
}

void App::on_r2_valueChanged(int value)
{
    ui->r2Label->setText(tr("r2 (") + QString::number(value) + tr(" /255)"));
}

void App::on_s2_valueChanged(int value)
{
    ui->s2Label->setText(tr("s2 (") + QString::number(value) + tr(" /255)"));
}

void App::on_sliderA_valueChanged(int value)
{
    ui->alabel->setText(tr("a (") + QString::number(value) + tr(" /255)"));
}

void App::on_sliderB_valueChanged(int value)
{
    ui->blabel->setText(tr("b (") + QString::number(value) + tr(" /255)"));
}

void App::on_sliderVal_valueChanged(int value)
{
    ui->valLabel->setText(tr("value (") + QString::number(value) + tr(" /255)"));
}

void App::on_btnG1_buttonPressed(QAbstractButton( *button))
{
    if (button == ui->rbtnHorizon) ui->matrixSegLine->setMatrix(hor);
    else if (button == ui->rbtnVertial) ui->matrixSegLine->setMatrix(ver);
    else if (button == ui->rbtn45) ui->matrixSegLine->setMatrix(p45);
    else if (button == ui->rbtnn45) ui->matrixSegLine->setMatrix(n45);
}

void App::on_comboBoxSeg_currentIndexChanged(int index)
{
    enum {THRES = 3};
    ui->stackedWidgetSeg->setCurrentIndex(index);
    if (index == THRES) ui->lbT->clear();
}

void App::on_comboBoxSegEdge_currentIndexChanged(int index)
{
    switch (index) {
    case SOBEL_SEG_EDGE:
        ui->matrixSegEdgeHor->setMatrix(solf1);
        ui->matrixSegEdgeVer->setMatrix(solf2);
        break;
    case ROBERT_SEG_EDGE:
        ui->matrixSegEdgeHor->setMatrix(rob1);
        ui->matrixSegEdgeVer->setMatrix(rob2);
        break;
    case PREWITT_SEG_EDGE:
        ui->matrixSegEdgeHor->setMatrix(prew1);
        ui->matrixSegEdgeVer->setMatrix(prew2);
        break;
    case LAPLACIAN_SEG_EDGE:
        ui->matrixSegEdgeHor->setMatrix(lapff);
        ui->matrixSegEdgeVer->setMatrix(zero);
        break;
    }
    ui->checkBoxVer->setHidden(index == LAPLACIAN_SEG_EDGE);
}

void App::on_comboBoxMor_currentIndexChanged(int index)
{
    enum {FILL = 5, FLOOD = 6};
    if (index == FILL || index == FLOOD){
        ui->statusBar->showMessage(tr("Click on image to select pixel to fill!!!"));
        ui->lbImage->setCursor(Qt::CursorShape::CrossCursor);
    } else {
        ui->lbImage->setCursor(Qt::CursorShape::ArrowCursor);
    }
}

void App::on_tabWidget_currentChanged(int index)
{
    enum {MOR = 3};
    enum {MOR_FILL = 5, MOR_FLOOD = 6};
    if (index == MOR && (ui->comboBoxMor->currentIndex() == MOR_FILL
                      || ui->comboBoxMor->currentIndex() == MOR_FLOOD)){
        ui->lbImage->setCursor(Qt::CursorShape::CrossCursor);
    } else {
        ui->lbImage->setCursor(Qt::CursorShape::ArrowCursor);
    }
}
