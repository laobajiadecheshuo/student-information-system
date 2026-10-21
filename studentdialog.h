#ifndef STUDENTDIALOG_H
#define STUDENTDIALOG_H

#include <QDialog>
#include <QtSql>
#include <QMap>

class QLineEdit;
class QComboBox;
class QDialogButtonBox;

class StudentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StudentDialog(QWidget *parent = nullptr);

    // 使用已有记录填充对话框（用于编辑）
    void setRecord(const QSqlRecord &rec);
    // 从对话框生成一个记录（用于插入/更新）
    QSqlRecord record() const;

    // 设置部门和专业的数据源（用于在打开数据库后设置）
    void setDepartments(const QMap<int, QString> &depts);
    void setMajors(const QMap<int, QString> &majors);

private slots:
    void onDepartmentChanged(int index);
    void validateAndAccept();

private:
    QLineEdit *leStudID;
    QLineEdit *leName;
    QComboBox *cbGender;
    QComboBox *cbDept;
    QComboBox *cbMajor;
    QDialogButtonBox *buttonBox;

    QMap<int, QString> m_depts;
    QMap<int, QString> m_majors;

    // helper to fill combos from maps
    void refillDeptCombo();
    void refillMajorCombo();
};

#endif // STUDENTDIALOG_H