#include "mywindow.h"
#include <QFileDialog>
#include <fstream>
#include <QHBoxLayout>
#include <QVBoxLayout>

Mywindow::Mywindow(QWidget * parent) : QMainWindow(parent){
    g = nullptr;
    this->ui();
    connect(open_button, SIGNAL(clicked()), this, SLOT(open()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(save()));
    connect(load_button, SIGNAL(clicked()), this, SLOT(load()));
    connect(parse_button, SIGNAL(clicked()), this, SLOT(parse()));
}

void Mywindow::ui(){
    setWindowTitle(tr("SLR(1)分析器"));
    this->iedit1 = new QPlainTextEdit(this);
    this->iedit2 = new QLineEdit(this);
    this->oedit1 = new QPlainTextEdit(this);
    this->oedit2 = new QPlainTextEdit(this);
    this->open_button = new QPushButton(tr("打开文法文件"), this);
    this->save_button = new QPushButton(tr("保存文法"), this);
    this->load_button = new QPushButton(tr("加载文法"), this);
    this->parse_button = new QPushButton(tr("分析句子"), this);
    //布局
    QVBoxLayout * button_layout = new QVBoxLayout();
    button_layout->addWidget(open_button);
    button_layout->addWidget(save_button);
    button_layout->addWidget(load_button);
    button_layout->addWidget(parse_button);
    QVBoxLayout * input_layout = new QVBoxLayout();
    input_layout->addWidget(iedit1);
    input_layout->addWidget(iedit2);
    QVBoxLayout * output_layout = new QVBoxLayout();
    output_layout->addWidget(oedit1);
    output_layout->addWidget(oedit2);   
    QWidget * cw = new QWidget();
    QHBoxLayout * layout = new QHBoxLayout(cw);
    layout->addLayout(input_layout);
    layout->addLayout(output_layout);
    layout->addLayout(button_layout);
    setCentralWidget(cw);
}

void Mywindow::open(){
    QString openfilename = QFileDialog::getOpenFileName(this,
    tr("Open File"), "", "", 0);
    if(!openfilename.isNull()){
        string filename = openfilename.toStdString();
        ifstream in(filename, ios::in);
        istreambuf_iterator<char> beg(in), end;
        string str(beg, end);
        in.close();
        this->iedit1->setPlainText(QString::fromStdString(str));   
    }
}

void Mywindow::save(){
    QString savefilename = QFileDialog::getSaveFileName(this,
    tr("Open Config"), "", "Config Files (*.tny)");
    if(!savefilename.isNull()){
        string filename = savefilename.toStdString();
        fstream out(filename, ios::out);
        out << iedit1->toPlainText().toStdString();
        out.close();
    }
}

void Mywindow::load(){
    if(g != nullptr){
        delete g;
    }
    g = new Grammar(this->iedit1->toPlainText().toStdString());
    this->oedit1->setPlainText(QString::fromStdString(this->g->show()));
}

void Mywindow::parse(){
    if(g == nullptr){
        this->oedit2->setPlainText(QString::fromStdString("文法为输入"));
        return;
    }
    this->oedit2->setPlainText(QString::fromStdString(this->g->parse(this->iedit2->text().toStdString())));
}