#ifndef MEDIATAGWRITER_H
#define MEDIATAGWRITER_H

#include <QString>
#include <QPixmap>
#include <QObject>
#include <taglib/taglib.h>
#include <taglib/mp4file.h>
#include <taglib/tfile.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

class MediaTagWriter : public QObject
{
    Q_OBJECT

public:
    explicit MediaTagWriter(QObject *parent = nullptr);
    bool writeTagsToFile(const QString& filePath, const QPixmap& coverArt);

signals:
    void progressUpdate(const QString& message);
    void error(const QString& message);
    void success(const QString& message);

private:
    bool writeMp4Tags(const QString& filePath, const QPixmap& coverArt);
    bool writeMkvTags(const QString& filePath, const QPixmap& coverArt);
    QByteArray pixmapToByteArray(const QPixmap& pixmap);
    bool isMkvpropeditAvailable();
    bool runMkvpropedit(const QString &mkvpropeditPath, const QString &movieFilePath, const QString &attachmentFilePath);
};

#endif // MEDIATAGWRITER_H
