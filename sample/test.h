
#ifndef TEST_H
#define TEST_H
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTimer>

class Objecttt : public QTableView{
    Q_OBJECT
public:
	QTableView *tableView;
	QStandardItemModel* model;
    Objecttt(QTableView *tableView, QStandardItemModel* model)
    {
    this->tableView = tableView;
    this->model = model;
    this->tableView->resize(850, 400);
    this->model->setHorizontalHeaderLabels({"ID", "Memory", "Tgid", "Name"});

    /* 自适应所有列，让它布满空间 */
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    /* 设置表格视图数据 */
    tableView->setModel(model);

    /* 显示 */
    tableView->show();

        connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
        m_timer.start(1000);
    }

public slots:
    void onTimeout();

private:
    QTimer m_timer;
};




#endif 

