#ifndef VPIECECARROUSEL_H
#define VPIECECARROUSEL_H

#include <QWidget>
#include <QComboBox>
#include <QScrollArea>

class VPieceCarrousel : public QWidget
{
    Q_OBJECT
public:
    explicit VPieceCarrousel(QWidget *parent = nullptr); 
    virtual ~VPieceCarrousel();

    void setOrientation(Qt::Orientation orientation);
signals:

public slots:

private:
    QComboBox *comboBoxLayer;
    QScrollArea *mainScrollArea;
    QList<QWidget *> layers;

private slots:
    void ActiveLayerChanged(int index);


};

#endif // VPIECECARROUSEL_H
