#ifndef     CONVERTER_H
#define     CONVERTER_H

#include    <QString>
#include    <QMap>

struct objects_ref_t
{
    QString model_name;
    QString texture_name;
};

class Converter
{
public:

    Converter(const QString &routeDir);
    ~Converter();

private:

    QString routeDirectory;

    QMap<QString, QString> model_names;
    QMap<QString, QString> texture_names;
    QMap<QString, objects_ref_t> objects_ref;

    void process(const QString &routeDir);

    bool readObjectsRef(const QString &path);
};

#endif // CONVERTER_H
