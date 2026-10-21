#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QDataWidgetMapper>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMap>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool openDatabase(const QString &aFile);

private slots:
    void do_currentChanged(const QModelIndex &current, const QModelIndex &previous);

    void on_actOpenDB_triggered();
    void on_actRecAppend_triggered();
    void on_actRecInsert_triggered();
    void on_actRevert_triggered();
    void on_actSubmit_triggered();
    void on_actRecDelete_triggered();
    void on_actFields_triggered();

    // 新增：搜索槽
    void on_searchClicked();

    // 双击/编辑记录
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_actRecEdit_triggered();

private:
    void openTable();    // 打开数据表
    void clearModels();  // 清理旧 model/selectionModel

private:
    Ui::MainWindow *ui;

    QSqlDatabase DB; // 数据库连接
    QString m_connName;

    QSqlRelationalTableModel *tabModel; // 数据模型
    QItemSelectionModel *selModel;      // 选择模型

    // 编辑按钮
    QAction *actRecEdit;

    // 搜索控件（添加到工具栏）
    QLineEdit *searchStudID;
    QLineEdit *searchName;
    QComboBox *searchDept;
    QPushButton *searchButton;

    // 部门/专业缓存（id->name）
    QMap<int, QString> m_departments;
    QMap<int, QString> m_majors;
};

#endif // MAINWINDOW_H
