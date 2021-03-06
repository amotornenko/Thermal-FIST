#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QTabWidget>

#include "modeltab.h"
#include "eventgeneratortab.h"
#include "energydependencetab.h"
#include "contourplottab.h"

#ifdef USE_MINUIT
#include "fittoexperimenttab.h"
#endif


//class TableModel;
class ThermalModelBase;
class ThermalParticleSystem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QTabWidget *tabWidget;

    ModelTab *tab1;
	
#ifdef USE_MINUIT
    FitToExperimentTab *tab2;
#endif

    EnergyDependenceTab *tab3;
    ContourPlotTab *tab4;
    EventGeneratorTab *tab5;

    QLineEdit *leDatabase;
    QPushButton *buttonLoad;
		QPushButton *buttonLoadDecays;
    //QTableView *tableParticles;

    ThermalParticleSystem *TPS;
    ThermalModelBase *model;

		QString cpath = "";

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void loadDatabase();
		void loadDecays();
};

#endif // MAINWINDOW_H
