#ifdef USE_MINUIT

#ifndef FITTOEXPERIMENTTAB_H
#define FITTOEXPERIMENTTAB_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QTableWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QElapsedTimer>
#include <QThread>

#include "BaseStructures.h"
#include "HRGFit/ThermalModelFit.h"

class FitWorker : public QThread
{
    Q_OBJECT

    ThermalModelFit *fTHMFit;

    void run() Q_DECL_OVERRIDE;

public:
    FitWorker(
           ThermalModelFit *THMFit=NULL,
           QObject * parent = 0) :
        QThread(parent), fTHMFit(THMFit)
    {
    }
signals:
    void calculated();
};


class QuantitiesModel;
class ThermalParticleSystem;
class ThermalModelBase;
class QElapsedTimer;

class FitToExperimentTab : public QWidget
{
    Q_OBJECT

    QTableView *tableQuantities;
    QPushButton *buttonAddQuantity, *buttonRemoveQuantity;
    QPushButton *buttonLoadFromFile;
    QTableWidget *tableParameters;

    //QRadioButton *radioIdeal, *radioEVMF, *radioEVMulti, *radioCE, *radioIdealCanonStrangeness, *radioIdealCanonCharm, *radioVDWHRG;
		//QRadioButton *radioEVCanonStrangeness, *radioVDWCanonStrangeness;

		QRadioButton *radIdeal, *radEVD, *radEVCRS, *radQVDW;
		QRadioButton *radGCE, *radCE, *radSCE;

		QRadioButton *radioBoltz, *radioQuant;
		QCheckBox *CBBoseOnly, *CBPionsOnly;
		QCheckBox *CBQuadratures;

    //QCheckBox *checkOnlyStable;
    QPushButton *buttonResults;
    QPushButton *buttonChi2Map;
		QPushButton *buttonChi2Profile;

    QDoubleSpinBox *spinTemperature, *spinmuB, *spingammaq, *spingammaS, *spinVolumeR;
    QCheckBox *CBTemperature, *CBmuB, *CBgammaq, *CBgammaS, *CBVolumeR;
    QDoubleSpinBox *spinTmin, *spinTmax;
    QDoubleSpinBox *spinmuBmin, *spinmuBmax;
    QDoubleSpinBox *spingqmin, *spingqmax;
    QDoubleSpinBox *spingsmin, *spingsmax;
    QDoubleSpinBox *spinVRmin, *spinVRmax;
    QSpinBox *spinB, *spinS, *spinQ;
    QDoubleSpinBox *spinQBRatio;
    QDoubleSpinBox *spinRadius;

		QCheckBox *checkFixMuQ, *checkFixMuS, *checkFixMuC;

    QCheckBox *checkFiniteWidth;
		QComboBox *comboWidth;

    QCheckBox *checkBratio;
	QCheckBox *checkFitRc;

    QCheckBox *checkOMP;

    QRadioButton *radioUniform, *radioBaglike, *radioMesons, *radioCustomEV;
		QString strEVPath;
    //QCheckBox *checkMesons;

    QPushButton *buttonCalculate;

    ThermalParticleSystem *TPS;
    ThermalModelBase *model;
    ThermalModelFit *fitcopy;

    QTextEdit *teDebug;

    QuantitiesModel *myModel;

    QElapsedTimer timer;
    QTimer *calcTimer;

		QString cpath;

    int getCurrentRow();

    std::vector<FittedQuantity> quantities;

public:
    FitToExperimentTab(QWidget *parent = 0, ThermalModelBase *model=NULL);
    ~FitToExperimentTab();
		ThermalModelConfig getConfig();
		ThermalModelFitParameters getFitParameters();
signals:

private slots:
    void writetofile();

public slots:
    void changedRow();
		void performFit(const ThermalModelConfig & config, const ThermalModelFitParameters & params);
    void calculate();
    //void benchmark();
    void quantityDoubleClick(const QModelIndex &);
    //void switchStability(bool);
    void showResults();
    void showChi2Map();
		void showChi2Profile();
    void setModel(ThermalModelBase *model);
    void removeQuantityFromFit();
    void addQuantity();
    void loadFromFile();
		void loadEVFromFile();
		void saveToFile();
    void modelChanged();
    void resetTPS();
    void updateProgress();
    void finalize();
};

#endif // FITTOEXPERIMENTTAB_H

#endif // USE_MINUIT
