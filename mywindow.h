#ifndef _MYWINDOW_H_
#define _MYWINDOW_H_

#include "grammar.h"
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>

class Mywindow : public QMainWindow{
    Q_OBJECT

private:
    QPlainTextEdit * iedit1, * oedit1, * oedit2; //输入框1、输出框1、输出框2
    QLineEdit * iedit2; //输入框2
    QPushButton * open_button;
    QPushButton * save_button;
    QPushButton * load_button;
    QPushButton * parse_button;
    Grammar * g;

public:
    explicit Mywindow(QWidget *parent = nullptr);
    void ui();

signals: //用来声明信号函数

public slots: //用来声明槽函数
    void open(); //打开文件
    void save(); //保存文件
    void load();  //载入文法于程序中
    void parse(); //分析句子
};

#endif