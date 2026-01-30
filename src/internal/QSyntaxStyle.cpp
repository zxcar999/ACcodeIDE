// QCodeEditor
#include "QSyntaxStyle.hpp"  // 👈 建议改为双引号包含自己的头文件

// Qt
#include <QDebug>
#include <QXmlStreamReader>
#include <QFile>

QSyntaxStyle::QSyntaxStyle(QObject* parent) :
    QObject(parent),
    m_name(),
    m_data(),
    m_loaded(false)
{
}

bool QSyntaxStyle::load(QString fl)
{
    QXmlStreamReader reader(fl);

    while (!reader.atEnd() && !reader.hasError())
    {
        auto token = reader.readNext();

        if(token == QXmlStreamReader::StartElement)
        {
            if (reader.name() == "style-scheme")
            {
                if (reader.attributes().hasAttribute("name"))
                {
                    m_name = reader.attributes().value("name").toString();
                }
            }
            else if (reader.name() == "style")
            {
                auto attributes = reader.attributes();
                auto name = attributes.value("name");

                QTextCharFormat format;

                if (attributes.hasAttribute("background"))
                {
                    format.setBackground(QColor(attributes.value("background").toString()));
                }

                if (attributes.hasAttribute("foreground"))
                {
                    format.setForeground(QColor(attributes.value("foreground").toString()));
                }

                if (attributes.hasAttribute("bold") &&
                    attributes.value("bold").toString() == "true")
                {
                    format.setFontWeight(QFont::Bold); // Qt6 兼容写法
                }

                if (attributes.hasAttribute("italic") &&
                    attributes.value("italic").toString() == "true")
                {
                    format.setFontItalic(true);
                }

                if (attributes.hasAttribute("underlineStyle"))
                {
                    auto underline = attributes.value("underlineStyle").toString();
                    auto s = QTextCharFormat::NoUnderline;

                    if (underline == "SingleUnderline")
                        s = QTextCharFormat::SingleUnderline;
                    else if (underline == "DashUnderline")
                        s = QTextCharFormat::DashUnderline;
                    else if (underline == "DotLine")
                        s = QTextCharFormat::DotLine;
                    else if (underline == "DashDotLine")
                        s = QTextCharFormat::DashDotLine;
                    else if (underline == "DashDotDotLine")
                        s = QTextCharFormat::DashDotDotLine;
                    else if (underline == "WaveUnderline")
                        s = QTextCharFormat::WaveUnderline;
                    else if (underline == "SpellCheckUnderline")
                        s = QTextCharFormat::SpellCheckUnderline;
                    else
                        qDebug() << "Unknown underline value:" << underline;

                    format.setUnderlineStyle(s);
                }

                m_data[name.toString()] = format;
            }
        }
    }

    m_loaded = !reader.hasError();
    return m_loaded;
}

QString QSyntaxStyle::name() const
{
    return m_name;
}

QTextCharFormat QSyntaxStyle::getFormat(QString name) const
{
    auto result = m_data.find(name);
    if (result == m_data.end())
        return QTextCharFormat();
    return result.value();
}

bool QSyntaxStyle::isLoaded() const
{
    return m_loaded;
}

QSyntaxStyle* QSyntaxStyle::defaultStyle()
{
    static QSyntaxStyle style;

    if (!style.isLoaded())
    {
        Q_INIT_RESOURCE(qcodeeditor_resources);
        QFile fl(":/default_style.xml");

        if (!fl.open(QIODevice::ReadOnly))
        {
            qWarning() << "Failed to open default_style.xml";
            return &style;
        }

        QByteArray data = fl.readAll();
        fl.close();

        if (!style.load(data))
        {
            qDebug() << "Can't load default style.";
        }
    }

    return &style;
}

// ✅✅✅ 新增：Dracula 主题 ✅✅✅
QSyntaxStyle* QSyntaxStyle::draculaStyle()
{
    auto style = new QSyntaxStyle();
    style->m_name = "Dracula";

    // 辅助 lambda：简化 QTextCharFormat 创建
    auto fmt = [](const QString& fg, const QString& bg = "", bool bold = false, bool italic = false) {
        QTextCharFormat f;
        if (!fg.isEmpty()) f.setForeground(QColor(fg));
        if (!bg.isEmpty()) f.setBackground(QColor(bg));
        if (bold) f.setFontWeight(QFont::Bold);
        if (italic) f.setFontItalic(true);
        return f;
    };

    // Dracula 官方配色: https://draculatheme.com/
    style->m_data["Text"]         = fmt("#f8f8f2", "#282a36");     // 普通文本 + 背景
    style->m_data["Keyword"]      = fmt("#ffff00", "", true);     // 关键字（粉色，加粗）
    style->m_data["Comment"]      = fmt("#6272a4");               // 注释（灰蓝）
    style->m_data["String"]       = fmt("#f1fa8c");               // 字符串（黄色）
    style->m_data["Number"]       = fmt("#bd93f9");               // 数字（紫色）
    style->m_data["Function"]     = fmt("#50fa7b");               // 函数（青绿）
    style->m_data["Type"]         = fmt("#8be9fd");               // 类型（如 int, void —— 青色）
    style->m_data["Preprocessor"] = fmt("#8be9fd");               // 预处理指令（#include 等）
    style->m_data["CurrentLine"]  = fmt("", "#44475a");           // 当前行背景
    style->m_data["Selection"]    = fmt("", "#44475a");           // 选中文本背景
    style->m_data["Parentheses"]  = fmt("#ff5555", "", true);

    style->m_loaded = true;
    return style;
}
