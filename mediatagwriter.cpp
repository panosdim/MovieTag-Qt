#include "mediatagwriter.h"
#include <QBuffer>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryFile>
#include <QResource>
#include <QPixmap>
#include <QImage>
#include <QDebug>

MediaTagWriter::MediaTagWriter(QObject *parent) : QObject(parent)
{
}

bool MediaTagWriter::writeTagsToFile(const QString& filePath, const QPixmap& coverArt)
{
    QString extension = QFileInfo(filePath).suffix().toLower();

    emit progressUpdate("Starting to write tags...");

    if (extension == "mp4") {
        return writeMp4Tags(filePath, coverArt);
    } else if (extension == "mkv") {
        return writeMkvTags(filePath, coverArt);
    } else {
        emit error("Unsupported file format");
        return false;
    }
}

QByteArray MediaTagWriter::pixmapToByteArray(const QPixmap& pixmap)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "JPEG");
    return imageData;
}

bool MediaTagWriter::writeMp4Tags(const QString& filePath, const QPixmap& coverArt)
{
    try {
        TagLib::MP4::File file(filePath.toStdString().c_str());
        if (!file.isValid()) {
            emit error("Invalid MP4 file");
            return false;
        }

        TagLib::MP4::Tag *tag = file.tag();
        if (tag) {
            // Add cover art
            QByteArray imageData = pixmapToByteArray(coverArt);
            TagLib::MP4::CoverArt::Format format = TagLib::MP4::CoverArt::JPEG;
            TagLib::ByteVector byteVector(imageData.data(), imageData.size());
            TagLib::MP4::CoverArt art(format, byteVector);

            // Create cover art list
            TagLib::MP4::CoverArtList coverArtList;
            coverArtList.append(art);

            // Get the current item list map and modify it
            TagLib::MP4::ItemMap items = tag->itemMap();
            // Remove existing cover art if present
            if (items.contains("covr")) {
                tag->removeItem("covr");
            }

            // Add new cover art
            tag->setItem("covr", coverArtList);  // Add or replace cover art

            emit progressUpdate("Saving MP4 tags...");
            if (file.save()) {
                emit success("MP4 tags written successfully");
                return true;
            }
        }

        emit error("Failed to write MP4 tags");
        return false;
    } catch (const std::exception& e) {
        emit error(QString("MP4 tagging error: %1").arg(e.what()));
        return false;
    }
}

bool MediaTagWriter::writeMkvTags(const QString& filePath, const QPixmap& coverArt)
{
    QString mkvpropeditPath;

    // Step 1: Check if mkvpropedit is available in the system
    if (isMkvpropeditAvailable()) {
        mkvpropeditPath = "mkvpropedit";  // System's mkvpropedit
    } else {
        emit error("mkvpropedit program not found. Please install it.");
        return false;
    }

    // Step 2: Convert QPixmap to a temporary image file (e.g., JPG)
    QTemporaryFile tempImageFile;
    tempImageFile.setAutoRemove(true);
    if (!tempImageFile.open()) {
        emit error("Failed to create temporary image file");
        return false;
    }

    // Convert QPixmap to QImage, then save to file
    QImage image = coverArt.toImage();
    if (!image.save(&tempImageFile, "JPEG")) {
        emit error("Failed to save image to temporary file in JPEG format");
        return false;
    }
    tempImageFile.flush();
    tempImageFile.close();


    emit progressUpdate("Saving MKV tags...");

    // Step 3: Run mkvpropedit to modify the MKV file
    if (!runMkvpropedit(mkvpropeditPath, filePath, tempImageFile.fileName())) {
        emit error("Failed to write MKV tags");
        return false;
    }

    emit success("MKV tags written successfully");
    return true;
}

bool MediaTagWriter::isMkvpropeditAvailable() {
    QProcess process;
    process.start("which", QStringList() << "mkvpropedit");
    process.waitForFinished();
    return process.exitCode() == 0;
}

bool MediaTagWriter::runMkvpropedit(const QString &mkvpropeditPath, const QString &movieFilePath, const QString &attachmentFilePath) {
    QProcess process;

    // Step 1: Delete existing attachments with MIME type "image/jpeg"
    QStringList deleteArgs;
    deleteArgs << movieFilePath << "--delete-attachment" << "mime-type:image/jpeg";
    process.start(mkvpropeditPath, deleteArgs);
    if (!process.waitForFinished()) {
        qWarning() << "Failed to execute mkvpropedit for deleting attachments:" << process.errorString();
        return false;
    }
    if (process.exitCode() == 2) {
        qWarning() << "Error occurred when trying to delete attachments.";
        return false;
    }

    // Step 2: Add new attachment
    QStringList addArgs;
    addArgs << movieFilePath
            << "--attachment-name" << "cover.jpg"
            << "--attachment-mime-type" << "image/jpeg"
            << "--add-attachment" << attachmentFilePath;
    process.start(mkvpropeditPath, addArgs);
    if (!process.waitForFinished()) {
        qWarning() << "Failed to execute mkvpropedit for adding attachment:" << process.errorString();
        return false;
    }
    if (process.exitCode() == 2) {
        qWarning() << "Error occurred when trying to add attachment.";
        return false;
    }

    return true;
}
