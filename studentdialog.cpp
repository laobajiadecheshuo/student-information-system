#include "studentdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QSqlRecord>
#include <QSqlField>
#include <QMessageBox>

StudentDialog::StudentDialog(QWidget *parent) :
    QDialog(parent),
    leStudID(new QLineEdit(this)),
    leName(new QLineEdit(this)),
    cbGender(new QComboBox(this)),
    cbDept(new QComboBox(this)),
    cbMajor(new QComboBox(this)),
    buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    setWindowTitle(tr("学生信息管理"));
    cbGender->addItem(tr("男"));
    cbGender->addItem(tr("女"));

    cbDept->addItem(QString());
    cbMajor->addItem(QString());

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("学号:"), leStudID);
    form->addRow(tr("姓名:"), leName);
    form->addRow(tr("性别:"), cbGender);
    form->addRow(tr("学院:"), cbDept);
    form->addRow(tr("专业:"), cbMajor);

    QVBoxLayout *mainLay = new QVBoxLayout;
    mainLay->addLayout(form);
    mainLay->addWidget(buttonBox);
    setLayout(mainLay);

    connect(cbDept, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StudentDialog::onDepartmentChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &StudentDialog::validateAndAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &StudentDialog::reject);
}

void StudentDialog::setDepartments(const QMap<int, QString> &depts)
{
    m_depts = depts;
    refillDeptCombo();
}

void StudentDialog::setMajors(const QMap<int, QString> &majors)
{
    m_majors = majors;
    refillMajorCombo();
}

void StudentDialog::refillDeptCombo()
{
    QVariant currentData = cbDept->currentData();
    cbDept->clear();
    cbDept->addItem(QString(), QVariant());
    for (auto it = m_depts.cbegin(); it != m_depts.cend(); ++it) {
        cbDept->addItem(it.value(), it.key());
    }
    if (currentData.isValid())
        cbDept->setCurrentIndex(cbDept->findData(currentData));
}

void StudentDialog::refillMajorCombo()
{
    QVariant currentData = cbMajor->currentData();
    cbMajor->clear();
    cbMajor->addItem(QString(), QVariant());
    for (auto it = m_majors.cbegin(); it != m_majors.cend(); ++it) {
        cbMajor->addItem(it.value(), it.key());
    }
    if (currentData.isValid())
        cbMajor->setCurrentIndex(cbMajor->findData(currentData));
}

void StudentDialog::onDepartmentChanged(int)
{
    // 如果需要基于学院过滤专业，这里可以实现。
}

void StudentDialog::setRecord(const QSqlRecord &rec)
{
    if (rec.isEmpty())
        return;
    leStudID->setText(rec.value("studID").toString());
    leName->setText(rec.value("name").toString());
    QString gender = rec.value("gender").toString();
    int gindex = cbGender->findText(gender);
    if (gindex >= 0) cbGender->setCurrentIndex(gindex);
    else cbGender->setCurrentIndex(0);

    QVariant deptv = rec.value("departID");
    if (deptv.isValid() && !deptv.isNull()) {
        int id = deptv.toInt();
        int idx = cbDept->findData(id);
        if (idx >= 0) cbDept->setCurrentIndex(idx);
    }

    QVariant majv = rec.value("majorID");
    if (majv.isValid() && !majv.isNull()) {
        int id = majv.toInt();
        int idx = cbMajor->findData(id);
        if (idx >= 0) cbMajor->setCurrentIndex(idx);
    }
}

QSqlRecord StudentDialog::record() const
{
    QSqlRecord rec;
    rec.append(QSqlField("studID", QVariant::String));
    rec.append(QSqlField("name", QVariant::String));
    rec.append(QSqlField("gender", QVariant::String));
    rec.append(QSqlField("departID", QVariant::Int));
    rec.append(QSqlField("majorID", QVariant::Int));

    rec.setValue("studID", leStudID->text());
    rec.setValue("name", leName->text());
    rec.setValue("gender", cbGender->currentText());

    QVariant deptData = cbDept->currentData();
    if (deptData.isValid())
        rec.setValue("departID", deptData.toInt());
    else
        rec.setNull("departID");

    QVariant majorData = cbMajor->currentData();
    if (majorData.isValid())
        rec.setValue("majorID", majorData.toInt());
    else
        rec.setNull("majorID");

    return rec;
}

void StudentDialog::validateAndAccept()
{
    if (leStudID->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("校验失败"), tr("学号不能为空"));
        return;
    }
    if (leName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("校验失败"), tr("姓名不能为空"));
        return;
    }
    accept();
}