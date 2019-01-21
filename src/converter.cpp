#include    "converter.h"

#include    <QDir>
#include    <QDirIterator>
#include    <QTextStream>

#include    <QDebug>

#include    <algorithm>
#include    <string>

#include    <QCoreApplication>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString toNativePath(const QString &path)
{
    QString tmp = path;

#if defined(Q_OS_UNIX)
    tmp.replace('\\', QDir::separator());
#else
    tmp.replace('/', QDir::separator());
#endif

    if (*(tmp.begin()) == QDir::separator())
        tmp.remove(0, 1);

    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Converter::Converter(const QString &routeDir)
{
    process(routeDir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Converter::~Converter()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Converter::process(const QString &routeDir)
{
    routeDirectory = routeDir;

    if (!QDir(routeDirectory).exists())
    {
        qDebug() << "Route is't found\n";
        return;
    }

    if (*(routeDir.end() - 1) != QDir::separator())
        routeDirectory += QDir::separator();    

    QString modelsDir = routeDirectory + "models";
    QString texturesDir = routeDirectory + "textures";

    QDir models(modelsDir);
    QDirIterator model_files(models.path(), QStringList() << "*.dmd", QDir::NoDotAndDotDot | QDir::Files);

    while (model_files.hasNext())
    {
        QFileInfo fileInfo(model_files.next());
        QString key = fileInfo.baseName().toLower();

        std::wstring tmp = fileInfo.fileName().toStdWString();
        size_t pos = tmp.find(L"х");

        QString tmp2;

        if (pos != std::wstring::npos)
        {
            tmp2 = QObject::tr(fileInfo.fileName().toStdString().c_str());
        }

        model_names.insert(key, fileInfo.fileName());
    }

    QDir textures(texturesDir);
    QDirIterator texture_files(textures.path(), QStringList() << "*.bmp" << "*.tga", QDir::NoDotAndDotDot | QDir::Files);

    while (texture_files.hasNext())
    {
        QFileInfo fileInfo(texture_files.next());
        QString key = fileInfo.fileName().toLower();

        texture_names.insert(key, fileInfo.fileName());
    }

    if (!readObjectsRef(routeDirectory + "objects.ref"))
    {
        qDebug() << "File objects.ref is't found\n";
        return;
    }

    rewriteObjectsRef();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Converter::readObjectsRef(const QString &path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QTextStream stream(&file);

    while (!stream.atEnd())
    {
        objects_ref_line_t ref_line;
        QString line = ref_line.content = stream.readLine();

        if (line.isEmpty() || (*(line.begin()) == ":") )
        {
            ref_line.type = EmptyLine;
            ref_lines.push_back(ref_line);
            continue;
        }

        if (*(line.begin()) == ';' )
        {
            ref_line.type = CommentLine;
            ref_lines.push_back(ref_line);
            continue;
        }

        if (*(line.begin()) == '[' )
        {
            ref_line.type = OptionLine;
            ref_lines.push_back(ref_line);
            continue;
        }

        ref_line.type = RefLine;
        ref_lines.push_back(ref_line);        
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Converter::rewriteObjectsRef()
{
    for (auto it = ref_lines.begin(); it != ref_lines.end(); ++it)
    {
        if (it->type == RefLine)
        {
            QStringList tokens = it->content.split('\t');
            QString new_content = "";

            for (auto token_it = tokens.begin() + 1; token_it != tokens.end(); ++token_it)
            {
                QFileInfo fileInfo(toNativePath(*token_it));
                QString key = "";

                if (token_it == tokens.begin() + 1)
                {
                    key = fileInfo.baseName().toLower();

                    auto model_it = model_names.find(key);

                    if (model_it.key() != key)
                        continue;

                    *token_it = QDir::separator() + fileInfo.path() + QDir::separator() + model_it.value();
                }
                else
                {
                    key = fileInfo.fileName().toLower();

                    auto texture_it = texture_names.find(key);

                    if (texture_it.key() != key)
                        continue;

                    *token_it = QDir::separator() + fileInfo.path() + QDir::separator() + texture_it.value();
                }
            }

            new_content += *(tokens.begin()) + "\t";

            for (auto token_it = tokens.begin() + 1; token_it != tokens.end() - 1; ++token_it)
                new_content += QDir::separator() + toNativePath(*token_it) + "\t";

            new_content += QDir::separator() + toNativePath(*(tokens.end() - 1));

            it->content = new_content;
        }
    }

    QFile::rename(routeDirectory + "objects.ref", routeDirectory + "objects.ref.bak");
    QFile refFile(routeDirectory + "objects.ref");

    if (refFile.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&refFile);

        for (auto it = ref_lines.begin(); it != ref_lines.end(); ++it)
        {
            stream << it->content << "\n";
        }

        refFile.close();
    }

    return true;
}
