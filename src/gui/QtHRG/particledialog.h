#ifndef PARTICLEDIALOG_H
#define PARTICLEDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QTableWidget>
#include <QPushButton>
#include <QTextEdit>
#include "tablemodel.h"

class ThermalModelBase;

class ParticleDialog : public QDialog
{
    Q_OBJECT

    //ThermalParticle *fParticle;
    //ThermalParticleSystem *fTPS;

    ThermalModelBase *model;
    int pid;


    QTextEdit *data;

    QTableView *tableDecays;
    QTableWidget *tableSources;
    DecayTableModel *myModel;

    QPushButton *buttonAddDecay;
    QPushButton *buttonRemoveDecay;

    QPushButton *buttonAddDaughterColumn;
    QPushButton *buttonRemoveDaughterColumn;

    QString GetParticleInfo();
public:
    explicit  ParticleDialog(QWidget *parent = 0, ThermalModelBase *mod = NULL, int ParticleID = -1);

signals:

public slots:
    void checkFixTableSize();
    void addDecay();
    void removeDecay();
    void addColumn();
    void removeColumn();
};

#endif // DECAYSEDITOR_H
