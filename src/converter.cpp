#include    "converter.h"

#include    <QDir>
#include    <QDirIterator>
#include    <QTextStream>

#include    <QDebug>

Converter::Converter(const QString &routeDir)
{
    process(routeDir);
}

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

    renameFiles();
}

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
bool Converter::readObjectsRef(const QString &path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QTextStream stream(&file);

    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;

        if (*(line.begin()) == ';' )
            continue;

        if (*(line.begin()) == '[' )
            continue;

        QStringList tokens = line.split('\t');

        if (tokens.size() < 3)
            continue;

        QFileInfo model(toNativePath(tokens[1]));
        QFileInfo texture(toNativePath(tokens[2]));

        objects_ref_t object_ref;

        object_ref.model_name = model.path() + QDir::separator() + model.fileName();
        object_ref.texture_name = texture.path() + QDir::separator() + texture.fileName();

        objects_ref.insert(model.baseName(), object_ref);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Converter::renameFiles()
{
    for (auto it = objects_ref.begin(); it != objects_ref.end(); ++it)
    {
        auto model_it = model_names.find(it.key());

        if (model_it.key() != it.key())
            continue;

        QString old_name = model_it.value();

        QFileInfo modelInfo(it.value().model_name);
        QFileInfo textureInfo(it.value().texture_name);

        QString oldModelName = routeDirectory + modelInfo.path() + QDir::separator() + old_name;
        QString newModelName = routeDirectory + it.value().model_name;

        if (oldModelName != newModelName)
        {
            QFile file(oldModelName);

            if (file.exists())
            {
                if (file.copy(newModelName))
                {
                    qDebug() << "Renamed from " << oldModelName << " to " << newModelName << "\n";
                    //QFile::remove(oldModelName);
                }
            }
        }

        auto texture_it = texture_names.find(textureInfo.fileName().toLower());

        if (texture_it.key() != textureInfo.fileName().toLower())
            continue;

        QString oldTextureName = routeDirectory + textureInfo.path() + QDir::separator() + texture_it.value();
        QString newTextureName = routeDirectory + it.value().texture_name;

        if (oldTextureName != newTextureName)
        {
            QFile file(oldTextureName);

            if (file.exists())
            {
                if (file.copy(newTextureName))
                {
                    qDebug() << "Renamed from " << oldTextureName << " to " << newTextureName << "\n";
                    //QFile::remove(oldTextureName);
                }
            }
        }
    }

    return true;
}
