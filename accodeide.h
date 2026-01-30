#ifndef ACCODEIDE_H
#define ACCODEIDE_H

#include <QMainWindow>
#include "D:\AppGallery\Qt\projects\ACcodeIDE\include\internal\QCodeEditor.hpp" // ← 新增：引入 CodeEditor
#include "include/internal/QCXXHighlighter.hpp"
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class ACcodeIDE; }
QT_END_NAMESPACE

class ACcodeIDE : public QMainWindow
{
    Q_OBJECT

public:
    ACcodeIDE(QWidget *parent = nullptr);
    ~ACcodeIDE();

private slots:
    void on_textEdit_textChanged();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void save();

    void codeappear();

    void on_pushButton_5_clicked();

    void open();

    void complete();

    void run();

    void updatetabname();

private:
    Ui::ACcodeIDE *ui;
    QCodeEditor *codeEditor; // ← 新增成员变量
    QCXXHighlighter* m_highlighter = nullptr;
    QProcess* m_compileProcess = nullptr; // 👈 新增成员
    QString m_currentExePath;
    QProcess* m_runProcess = nullptr;

};

#endif // ACCODEIDE_H
