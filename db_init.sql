-- 学生信息管理系统 数据库初始化脚本
-- 用法: sqlite3 infodb.db3 < db_init.sql

CREATE TABLE IF NOT EXISTS departments (
    departID   INTEGER PRIMARY KEY,
    department VARCHAR(50) NOT NULL);

CREATE TABLE IF NOT EXISTS majors (
    majorID INTEGER PRIMARY KEY,
    major   VARCHAR(50) NOT NULL);

CREATE TABLE IF NOT EXISTS studInfo (
    studID   VARCHAR(20) PRIMARY KEY,
    name     VARCHAR(20) NOT NULL,
    gender   VARCHAR(4),
    departID INTEGER REFERENCES departments(departID),
    majorID  INTEGER REFERENCES majors(majorID));

-- 示例数据
INSERT OR IGNORE INTO departments VALUES (1, '信息与通信工程学院');
INSERT OR IGNORE INTO departments VALUES (2, '计算机科学与工程学院');
INSERT OR IGNORE INTO majors VALUES (1, '通信工程');
INSERT OR IGNORE INTO majors VALUES (2, '计算机科学与技术');
INSERT OR IGNORE INTO studInfo VALUES
    ('2025091601001', '张三', '男', 1, 1),
    ('2025091601002', '李四', '女', 2, 2);
