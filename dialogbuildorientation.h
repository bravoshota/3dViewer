#pragma once

#include <QDialog>
#include "common.h"

namespace Ui {
class DialogBuildOrientation;
}

class DialogBuildOrientation : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBuildOrientation(QWidget *parent, common::Vertex &rotation);
    ~DialogBuildOrientation();

    void provideValues();

private:
    Ui::DialogBuildOrientation *ui;
    common::Vertex &m_rotation;
};
