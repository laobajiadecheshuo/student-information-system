# 学生信息管理系统（Information）

电子科技大学《开源软件》课程项目。基于 **Qt Widgets + Qt SQL（QSqlRelationalTableModel）+ SQLite** 的桌面学生信息管理工具。

## 功能

- 学生信息增删改查（添加 / 插入 / 删除 / 编辑对话框，手动提交 / 撤销）
- 学院、专业两个外键字段通过 `QSqlRelation` + `QSqlRelationalDelegate` 映射为下拉选择
- 组合搜索：按学号（模糊）、姓名（模糊）、学院（下拉）
- 打开任意 `.db3` SQLite 数据库文件

## 构建（Windows / Qt 6）

1. 安装 [Qt 6](https://www.qt.io/download)（MSVC 套件）与 Visual Studio
2. 用 Qt 命令行或直接运行：

```bat
qmake information.pro
nmake            # 或用 Qt Creator 打开 information.pro 直接构建
```

3. 产物为 `release/information.exe`（或 Qt Creator 的构建目录）

## 数据库表结构

`studInfo(studID, name, gender, departID→departments, majorID→majors)`

## 目录说明

| 文件 | 说明 |
|---|---|
| `mainwindow.cpp` | 主窗口：模型/搜索工具栏/CRUD |
| `studentdialog.cpp` | 学生信息编辑对话框（含字段校验） |
| `information.pro` | qmake 工程文件 |
