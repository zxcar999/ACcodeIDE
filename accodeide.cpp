#include "accodeide.h"
#include "ui_accodeide.h"
#include "include/internal/QCXXHighlighter.hpp"
#include "include/internal/QSyntaxStyle.hpp"
#include <qfile.h>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <qprocess.h>
#include <QScrollBar>
#include <QTemporaryFile>
#include <QTimer>
#include <quuid.h>
#include <qdatetime.h>

QString code="";
QString filepath;
bool saved=false;
bool newfile=true;
int TabReplaceSize=4;
int completeIndex=0;
int runIndex=1;

// accodeide.cpp - 构造函数部分
ACcodeIDE::ACcodeIDE(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ACcodeIDE)
{
    ui->setupUi(this);

    QCodeEditor* editor = qobject_cast<QCodeEditor*>(ui->textEdit);
    if (!editor) {
        qWarning() << "Failed to cast textEdit to QCodeEditor!";
        return;
    }

    // 设置字体
    QFont font("Source Code Pro");
    font.setPixelSize(14);
    font.setFixedPitch(true);
    editor->setFont(font);
    editor->setLineWrapMode(QTextEdit::NoWrap);

    // ✅ 正确创建并保存高亮器
    m_highlighter = new QCXXHighlighter(editor->document());

    editor->setSyntaxStyle(QSyntaxStyle::defaultStyle());

    // 其他设置
    editor->setAutoIndentation(true);
    editor->setAutoParentheses(true);
    editor->setTabReplace(true);
    editor->setTabReplaceSize(TabReplaceSize);

    // 输出框只读
    ui->plainTextEdit->setReadOnly(true);
    ui->plainTextEdit_2->setReadOnly(true);
    ui->tabWidget_2->setCurrentIndex(completeIndex);
}

ACcodeIDE::~ACcodeIDE()
{
    delete ui;
}

void ACcodeIDE::run()
{
    ui->tabWidget_2->setCurrentIndex(runIndex);
    if (filepath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先打开或保存一个 C++ 文件");
        return;
    }

    QFileInfo srcInfo(filepath);
    QString exe = QDir::toNativeSeparators(srcInfo.absolutePath() + "/" + srcInfo.baseName() + ".exe");
    if (!QFile::exists(exe)) {
        QMessageBox::warning(this, "错误", "可执行文件不存在，请先编译！");
        return;
    }
    ui->plainTextEdit_2->appendPlainText(QString(QDateTime::currentDateTime().toString()+" 程序开始运行"));
    ui->plainTextEdit_2->verticalScrollBar()->setValue(ui->plainTextEdit_2->verticalScrollBar()->maximum());

    // 生成唯一临时文件名（避免冲突）
    QString id = QUuid::createUuid().toString(QUuid::Id128);
    QString exitFile = QDir::temp().filePath(srcInfo.baseName() + "_exit_" + id + ".txt");
    QString batPath  = QDir::temp().filePath(srcInfo.baseName() + "_run_"  + id + ".bat");

    // 构造批处理：运行 exe -> 保存 exitCode -> 显示 Exit code -> pause -> 写 exitFile -> exit /b exitCode
    // 注意：将写入 exitFile 放在 pause 之后，避免 Qt 删除 bat 导致 "找不到批处理文件" 的 race。
    QString batContent;
    batContent += "@echo off\r\n";
    batContent += "\"" + exe + "\"\r\n"; // 运行可执行程序（用双引号防止路径中空格问题）
    batContent += "set exitCode=%ERRORLEVEL%\r\n";
    batContent += "echo.\r\n";
    batContent += "echo ____________________________\r\n";
    batContent += "echo Exit code: %exitCode%\r\n";
    batContent += "pause\r\n";
    batContent += "echo %exitCode% > \"" + QDir::toNativeSeparators(exitFile) + "\"\r\n";
    batContent += "exit /b %exitCode%\r\n";

    QFile bf(batPath);
    if (!bf.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建临时批处理文件：" + bf.errorString());
        return;
    }
    bf.write(batContent.toLocal8Bit());
    bf.close();

    // 启动批处理（新窗口）
    QString quotedBat = QString("%1").arg(QDir::toNativeSeparators(batPath));
    QStringList args;
    args << "/c" << "start" << "" << "cmd" << "/c" << quotedBat;
    if (!QProcess::startDetached("cmd", args)) {
        QMessageBox::critical(this, "运行失败", "无法启动控制台窗口");
        QFile::remove(batPath);
        return;
    }

    // 轮询 exitFile — 当用户在控制台按任意键后 bat 会写出 exitFile，Qt 读取并清理
    QTimer *timer = new QTimer(this);
    timer->setInterval(400); // 每 400 ms 检查一次
    connect(timer, &QTimer::timeout, this, [this, exitFile, batPath, timer]() {
        if (QFile::exists(exitFile)) {
            timer->stop();

            QString codeStr = "无法读取";
            quint32 codeInt=0;
            QString codeHex="Nan";
            QFile f(exitFile);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                codeStr = QString::fromLocal8Bit(f.readAll()).trimmed();
                f.close();
                codeInt = static_cast<quint32>(codeStr.toInt());
                codeHex = QString("0x%1").arg(codeInt, 8, 16, QChar('0')).toUpper();
            }
            ui->plainTextEdit_2->appendPlainText(QString(QDateTime::currentDateTime().toString()+" 程序运行结束，退出码：%1("+codeHex+")").arg(codeStr));
            ui->plainTextEdit_2->verticalScrollBar()->setValue(ui->plainTextEdit_2->verticalScrollBar()->maximum());

            // 清理临时文件
            QFile::remove(exitFile);
            QFile::remove(batPath);
            timer->deleteLater();
        }
    });
    timer->start();
}
void ACcodeIDE::complete()
{
    ui->tabWidget_2->setCurrentIndex(completeIndex);
    ui->plainTextEdit->clear();
    if (filepath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先打开或保存一个 C++ 文件");
        return;
    }
    if (filepath.isEmpty()) return;
    if (!saved && !filepath.isEmpty()) {
        save();
        if (filepath.isEmpty()) return;
    }

    QString appDir = QCoreApplication::applicationDirPath();
    QString compiler = appDir + "/TDM-GCC-64/bin/g++.exe";

    ui->plainTextEdit->appendPlainText(QString(QDateTime::currentDateTime().toString()+" 编译中..."));
    ui->plainTextEdit->verticalScrollBar()->setValue(
        ui->plainTextEdit->verticalScrollBar()->maximum()
    );

    if (!QFile::exists(compiler)) {
        ui->plainTextEdit->appendPlainText("错误: 编译器未找到！\n" + compiler);
        return;
    }

    QFileInfo srcInfo(filepath);
    m_currentExePath = srcInfo.absolutePath() + "/" + srcInfo.baseName() + ".exe";

    QStringList args;
    args << filepath
         << "-o" << m_currentExePath
         << "-std=c++14"
         << "-static-libgcc";

    // ✅ 使用成员变量，避免析构
    if (m_compileProcess) {
        m_compileProcess->kill();
        m_compileProcess->deleteLater();
    }
    m_compileProcess = new QProcess(this);

    // 连接信号
    connect(m_compileProcess, &QProcess::readyReadStandardError, this, [this]() {
        QByteArray err = m_compileProcess->readAllStandardError();
        ui->plainTextEdit->appendPlainText(QString::fromLocal8Bit(err));
        ui->plainTextEdit->verticalScrollBar()->setValue(
            ui->plainTextEdit->verticalScrollBar()->maximum()
            );
    });

    connect(m_compileProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus);
                if (exitCode == 0) {
                    ui->plainTextEdit->appendPlainText(QString(QDateTime::currentDateTime().toString()+" 编译成功！"));
                } else {
                    ui->plainTextEdit->appendPlainText(QString(QDateTime::currentDateTime().toString()+" 编译失败（退出码：" + QString::number(exitCode) + "）"));
                }
                ui->plainTextEdit->verticalScrollBar()->setValue(
                    ui->plainTextEdit->verticalScrollBar()->maximum()
                    );
                m_compileProcess->deleteLater();
                m_compileProcess = nullptr;
            });

    // 启动编译
    m_compileProcess->start(compiler, args);

    if (!m_compileProcess->waitForStarted(3000)) {
        ui->plainTextEdit->appendPlainText("无法启动编译器");
        m_compileProcess->deleteLater();
        m_compileProcess = nullptr;
    }

}

void ACcodeIDE::on_textEdit_textChanged()
{
    if (!ui || !ui->textEdit) return;
    saved=false;
    QCodeEditor* editor = qobject_cast<QCodeEditor*>(ui->textEdit);
    if (!editor) {
        qWarning() << "textEdit is not a QCodeEditor!";
        return;
    }

    code = editor->toPlainText(); // ✅ 安全使用
}

void ACcodeIDE::codeappear()
{
    QCodeEditor* editor = qobject_cast<QCodeEditor*>(ui->textEdit);
    if (!editor) return;
    editor->setPlainText(code);
}

void ACcodeIDE::save()
{
    if(newfile){
        filepath = QFileDialog::getSaveFileName(
            this,
            "保存",
            QDir::homePath(),
            "C++ Files (*.cpp *.h *.hpp *.cc);;All Files(*)"
            );
        if(filepath.isEmpty()){
            return;
        }
    }
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法保存文件:" << file.errorString();
        return;
    }
    QCodeEditor* editor = qobject_cast<QCodeEditor*>(ui->textEdit);
    if (!editor) return;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << editor->toPlainText();
    file.close();

    updatetabname();

    qDebug() << "文件已保存:" << filepath;
    saved=true;
    newfile=false;
    return;
}

void ACcodeIDE::open(){


    filepath = QFileDialog::getOpenFileName(
        this,
        "打开文件",
        QDir::homePath(),
        "C++ Files (*.cpp *.h *.hpp *.cc);;All Files (*)"
        );

    if (filepath.isEmpty()) {
        return;
    }

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件:\n" + file.errorString());
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();

    QCodeEditor* editor = qobject_cast<QCodeEditor*>(ui->textEdit);
    if (editor) {
        editor->setPlainText(content);
    }

    updatetabname();

    code = content;
    saved = true;
    newfile=false;
    setWindowTitle("ACcodeIDE - " + QFileInfo(filepath).fileName());
}

void ACcodeIDE::updatetabname(){
    int currentIndex = ui->tabWidget->currentIndex(); // 获取当前标签页索引
    if (currentIndex != -1) {
        QString fileName = QFileInfo(filepath).fileName(); // 得到 "hello.cpp"
        ui->tabWidget->setTabText(currentIndex, fileName);
    }
}

void ACcodeIDE::on_pushButton_clicked()
{
    complete();
}


void ACcodeIDE::on_pushButton_2_clicked()
{
    run();
}


void ACcodeIDE::on_pushButton_3_clicked()
{
    save();
    return;
}


void ACcodeIDE::on_pushButton_4_clicked()
{
    if(saved){
        saved=false;
        code="";
        QCodeEditor* editor = qobject_cast<QCodeEditor*>(ui->textEdit);
        if (!editor) {
            qWarning() << "textEdit is not a QCodeEditor!";
            return;
        }
        codeappear();
        return;
    }else{
        int WarningMassage=QMessageBox::warning(this, "警告", "文件未保存，确定要保存吗？",QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (WarningMassage == QMessageBox::Yes){
            save();
        } else if (WarningMassage == QMessageBox::No){

        } else {
            return;
        }
        code="";
        codeappear();
    }
    setWindowTitle("ACcodeIDE");
    return;
}


void ACcodeIDE::on_pushButton_5_clicked()
{
    if(!saved){
        int WarningMassage=QMessageBox::warning(this, "警告", "文件未保存，打开将关闭"+filepath+"确定要保存吗？",QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (WarningMassage == QMessageBox::Yes){
            save();
        } else if (WarningMassage == QMessageBox::No){

        } else {
            return;
        }
    }
    open();
    saved=true;
    return;
}

