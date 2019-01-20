
#include "ChartWidget.h"

#include <ui_ChartWidget.h>

namespace MoonGlare::PerfView {

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent) {
    ui = std::make_unique<Ui::ChartWidget>();
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, [this] {
        OnCloseButtonPressed(this);
    });
}

ChartWidget::~ChartWidget() {
    ui.reset();
}

void ChartWidget::AddChart(QWidget *widget) {
    ui->content->addWidget(widget);
}

}
