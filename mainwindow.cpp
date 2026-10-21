#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "studentdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// 清理旧模型和选择模型（如果存在）
void MainWindow::clearModels()
{
    if (ui->tableView->model() == tabModel) {
        ui->tableView->setModel(nullptr);
    }
    if (tabModel) {
        delete tabModel;
        tabModel = nullptr;
    }
    if (ui->tableView->selectionModel() == selModel) {
        ui->tableView->setSelectionModel(nullptr);
    }
    if (selModel) {
        delete selModel;
        selModel = nullptr;
    }
}

// 打开数据表
void MainWindow::openTable()
{
    // 清理旧 model（如果有）
    clearModels();

    tabModel = new QSqlRelationalTableModel(this, DB);
    tabModel->setTable("studInfo");     // 设置数据表
    tabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);  // 编辑策略
    tabModel->setSort(tabModel->fieldIndex("studID"), Qt::AscendingOrder);

    selModel = new QItemSelectionModel(tabModel, this);     // 创建选择模型
    connect(selModel, &QItemSelectionModel::currentChanged, this, &MainWindow::do_currentChanged);

    ui->tableView->setModel(tabModel);
    ui->tableView->setSelectionModel(selModel);

    tabModel->setHeaderData(tabModel->fieldIndex("studID"),  Qt::Horizontal, "学号");
    tabModel->setHeaderData(tabModel->fieldIndex("name"),    Qt::Horizontal, "姓名");
    tabModel->setHeaderData(tabModel->fieldIndex("gender"),  Qt::Horizontal, "性别");
    tabModel->setHeaderData(tabModel->fieldIndex("departID"),Qt::Horizontal, "学院");
    tabModel->setHeaderData(tabModel->fieldIndex("majorID"), Qt::Horizontal, "专业");

    // 设置关系字段
    tabModel->setRelation(tabModel->fieldIndex("departID"),
                          QSqlRelation("departments", "departID", "department")); // 学院
    tabModel->setRelation(tabModel->fieldIndex("majorID"),
                          QSqlRelation("majors", "majorID", "major"));            // 专业

    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));  // 关系字段代理

    if (!tabModel->select()) {
        QMessageBox::warning(this, tr("错误"), tr("查询数据失败: %1").arg(tabModel->lastError().text()));
    }

    // 从数据库加载 departments 和 majors 到缓存（用于搜索下拉和对话框）
    m_departments.clear();
    m_majors.clear();
    QSqlQuery q(DB);
    if (q.exec("SELECT departID, department FROM departments")) {
        while (q.next()) {
            int id = q.value(0).toInt();
            QString name = q.value(1).toString();
            m_departments.insert(id, name);
        }
    } else {
        qDebug() << "load departments failed:" << q.lastError().text();
    }

    if (q.exec("SELECT majorID, major FROM majors")) {
        while (q.next()) {
            int id = q.value(0).toInt();
            QString name = q.value(1).toString();
            m_majors.insert(id, name);
        }
    } else {
        qDebug() << "load majors failed:" << q.lastError().text();
    }

    // 填充搜索下拉
    searchDept->clear();
    searchDept->addItem(QString(), QVariant()); // 空项
    for (auto it = m_departments.cbegin(); it != m_departments.cend(); ++it)
        searchDept->addItem(it.value(), it.key());

    ui->actOpenDB->setEnabled(false);
    ui->actRecAppend->setEnabled(true);
    ui->actRecInsert->setEnabled(true);
    ui->actRecDelete->setEnabled(true);
    actRecEdit->setEnabled(true);
    ui->actFields->setEnabled(true);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_connName("sampConn"),
    tabModel(nullptr),
    selModel(nullptr),
    actRecEdit(nullptr),
    searchStudID(nullptr),
    searchName(nullptr),
    searchDept(nullptr),
    searchButton(nullptr)
{
    ui->setupUi(this);

    // 主工具栏只显示文字，不显示图标
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    this->setCentralWidget(ui->tableView);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setAlternatingRowColors(true);
    // 移除 DoubleClicked 编辑触发，避免与自定义编辑对话框冲突
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 搜索工具栏（位于主工具栏下方）
    QToolBar *searchBar = new QToolBar(tr("搜索"), this);
    searchBar->setMovable(false);
    searchBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, searchBar);

    QLabel *lab1 = new QLabel(tr(" 学号: "), this);
    searchStudID = new QLineEdit(this);
    searchStudID->setMaximumWidth(120);
    searchStudID->setPlaceholderText(tr("输入学号"));
    searchBar->addWidget(lab1);
    searchBar->addWidget(searchStudID);

    QLabel *lab2 = new QLabel(tr(" 姓名: "), this);
    searchName = new QLineEdit(this);
    searchName->setMaximumWidth(120);
    searchName->setPlaceholderText(tr("输入姓名"));
    searchBar->addWidget(lab2);
    searchBar->addWidget(searchName);

    QLabel *lab3 = new QLabel(tr(" 学院: "), this);
    searchDept = new QComboBox(this);
    searchDept->setMaximumWidth(160);
    searchBar->addWidget(lab3);
    searchBar->addWidget(searchDept);

    searchButton = new QPushButton(tr("🔍 搜索"), this);
    searchBar->addWidget(searchButton);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::on_searchClicked);

    // 清除搜索按钮
    QPushButton *clearBtn = new QPushButton(tr("✕ 清除"), this);
    searchBar->addWidget(clearBtn);
    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        searchStudID->clear();
        searchName->clear();
        searchDept->setCurrentIndex(0);
        if (tabModel) {
            tabModel->setFilter("");
            tabModel->select();
        }
    });

    // 编辑按钮（工具栏）
    actRecEdit = new QAction(tr("编辑"), this);
    actRecEdit->setEnabled(false);
    actRecEdit->setToolTip(tr("编辑当前选中记录"));
    ui->mainToolBar->insertAction(ui->actSubmit, actRecEdit);
    connect(actRecEdit, &QAction::triggered, this, &MainWindow::on_actRecEdit_triggered);

    // 双击编辑 — 槽名 on_tableView_doubleClicked 已被 setupUi 中的
    // connectSlotsByName 自动连接，此处无需（也不应）重复 connect
}

MainWindow::~MainWindow()
{
    clearModels();
    delete ui;
}

// 打开/切换数据库（安全地移除旧连接并创建新连接）
bool MainWindow::openDatabase(const QString &aFile)
{
    // 如果连接已存在且打开，先关闭并移除
    if (QSqlDatabase::contains(m_connName)) {
        QSqlDatabase old = QSqlDatabase::database(m_connName);
        if (old.isOpen())
            old.close();
        QSqlDatabase::removeDatabase(m_connName);
    }

    DB = QSqlDatabase::addDatabase("QSQLITE", m_connName);
    DB.setDatabaseName(aFile);
    if (DB.open()) {
        openTable();    // 打开数据表
        return true;
    } else {
        return false;
    }
}

// 更新 actSubmit 和 actRevert 的状态
void MainWindow::do_currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
    if (tabModel)
    {
        ui->actSubmit->setEnabled(tabModel->isDirty()); // 有未保存的修改时可用
        ui->actRevert->setEnabled(tabModel->isDirty());
    }
}

void MainWindow::on_actOpenDB_triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, "选择文件", "", "SQLite数据库(*.db3)");
    if (aFile.isEmpty())
        return;

    if (!openDatabase(aFile)) {
        QMessageBox::warning(this, "错误", "打开数据库失败");
    }
}

void MainWindow::on_actRecAppend_triggered()
{ // 添加记录 - 弹出学生对话框
    if (!tabModel) return;

    StudentDialog dlg(this);
    dlg.setDepartments(m_departments);
    dlg.setMajors(m_majors);

    if (dlg.exec() == QDialog::Accepted) {
        QSqlRecord rec = dlg.record();
        int row = tabModel->rowCount();
        bool ok = tabModel->insertRecord(row, rec);
        if (!ok) {
            QMessageBox::warning(this, tr("错误"), tr("插入记录失败: %1").arg(tabModel->lastError().text()));
        } else {
            ui->actSubmit->setEnabled(true);
            ui->actRevert->setEnabled(true);
            // 选中新插入行
            ui->tableView->selectRow(row);
        }
    }
}

void MainWindow::on_actRecInsert_triggered()
{ // 在当前行之前插入记录 - 弹出对话框
    if (!tabModel) return;

    QModelIndex curIndex = ui->tableView->currentIndex();
    int row = curIndex.isValid() ? curIndex.row() : tabModel->rowCount();

    StudentDialog dlg(this);
    dlg.setDepartments(m_departments);
    dlg.setMajors(m_majors);

    if (dlg.exec() == QDialog::Accepted) {
        QSqlRecord rec = dlg.record();
        bool ok = tabModel->insertRecord(row, rec);
        if (!ok) {
            QMessageBox::warning(this, tr("错误"), tr("插入记录失败: %1").arg(tabModel->lastError().text()));
        } else {
            ui->actSubmit->setEnabled(true);
            ui->actRevert->setEnabled(true);
            ui->tableView->selectRow(row);
        }
    }
}

void MainWindow::on_actRecDelete_triggered()
{ // 删除当前行（标记为删除，手动提交）
    if (!tabModel || !selModel) return;
    int row = selModel->currentIndex().row();
    if (row < 0) return;
    tabModel->removeRow(row);
    ui->actSubmit->setEnabled(true);
    ui->actRevert->setEnabled(true);
}

void MainWindow::on_actSubmit_triggered()
{ // 保存修改
    if (!tabModel) return;
    bool res = tabModel->submitAll();
    if (!res)
        QMessageBox::information(this, "消息", "数据保存错误,错误信息\n" + tabModel->lastError().text());
    else
    {
        ui->actSubmit->setEnabled(false);
        ui->actRevert->setEnabled(false);
    }
}

void MainWindow::on_actRevert_triggered()
{ // 取消修改
    if (!tabModel) return;
    tabModel->revertAll();
    ui->actSubmit->setEnabled(false);
    ui->actRevert->setEnabled(false);
}

void MainWindow::on_actFields_triggered()
{ // 获取字段列表
    if (!tabModel) return;
    QSqlRecord emptyRec = tabModel->record(); // 获取空记录，只有字段名
    QString str;
    for (int i = 0; i < emptyRec.count(); ++i) {
        str += emptyRec.fieldName(i) + '\n';
    }
    QMessageBox::information(this, "所有字段名", str);
}

// 搜索实现（对输入做简单转义以避免 SQL 错误）
void MainWindow::on_searchClicked()
{
    if (!tabModel) return;

    QStringList filters;
    QString id = searchStudID->text().trimmed();
    QString name = searchName->text().trimmed();
    QVariant deptData = searchDept->currentData();

    if (!id.isEmpty()) {
        QString idEsc = id;
        idEsc.replace("'", "''");
        filters << QString("studID LIKE '%1'").arg("%" + idEsc + "%");
    }
    if (!name.isEmpty()) {
        QString nameEsc = name;
        nameEsc.replace("'", "''");
        filters << QString("name LIKE '%1'").arg("%" + nameEsc + "%");
    }
    if (deptData.isValid()) {
        filters << QString("studInfo.departID=%1").arg(deptData.toInt());
    }

    QString filter = filters.join(" AND ");
    tabModel->setFilter(filter);
    if (!tabModel->select()) {
        qDebug() << "filter select failed:" << tabModel->lastError().text();
    }
}

// 双击编辑（通过对话框修改 model，不自动提交）
void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid() || !tabModel) return;

    int row = index.row();
    QSqlRecord rec = tabModel->record(row);

    // 关系字段 record() 返回的是显示文字（如"计算机学院"），
    // 用 EditRole 获取真正的外键 ID，否则对话框匹配不到
    int deptCol = tabModel->fieldIndex("departID");
    int majorCol = tabModel->fieldIndex("majorID");
    rec.setValue("departID", tabModel->index(row, deptCol).data(Qt::EditRole));
    rec.setValue("majorID", tabModel->index(row, majorCol).data(Qt::EditRole));

    StudentDialog dlg(this);
    dlg.setDepartments(m_departments);
    dlg.setMajors(m_majors);
    dlg.setRecord(rec);

    if (dlg.exec() == QDialog::Accepted) {
        QSqlRecord newrec = dlg.record();
        // 使用 setRecord 直接写底层表，避免 QSqlRelationalTableModel::setData
        // 对关系字段（departID/majorID）静默失败
        tabModel->setRecord(row, newrec);

        ui->actSubmit->setEnabled(true);
        ui->actRevert->setEnabled(true);
    }
}

void MainWindow::on_actRecEdit_triggered()
{
    if (!selModel) return;
    QModelIndex idx = selModel->currentIndex();
    if (idx.isValid())
        on_tableView_doubleClicked(idx);
}