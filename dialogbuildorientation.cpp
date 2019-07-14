#include "dialogbuildorientation.h"
#include "ui_dialogbuildorientation.h"

DialogBuildOrientation::DialogBuildOrientation(QWidget *parent, common::Vertex &rotation) :
    QDialog(parent),
    ui(new Ui::DialogBuildOrientation),
    m_rotation(rotation)
{
    setWindowModality(Qt::WindowModality::WindowModal);
    ui->setupUi(this);
    ui->spinBoxDirectionX->setValue(m_rotation.x);
    ui->spinBoxDirectionY->setValue(m_rotation.y);
    ui->spinBoxDirectionZ->setValue(m_rotation.z);
}

DialogBuildOrientation::~DialogBuildOrientation()
{
    delete ui;
}

void DialogBuildOrientation::provideValues()
{
    m_rotation.x = ui->spinBoxDirectionX->value();
    m_rotation.y = ui->spinBoxDirectionY->value();
    m_rotation.z = ui->spinBoxDirectionZ->value();
}
