#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QLabel>
#include <QProcess>
#include "tmdbclient.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenMovieButtonClick();
    void onSearchButtonClick();
    void onWriteTagsButtonClick();
    void onSearchCompleted(const QJsonArray& results);
    void onSearchResultSelectionChanged();

private:
    // Enum to define message types
    enum class MessageType {
        Error,
        Warning,
        Normal
    };

    // Function to read the configuration file
    void readConfigFile();

    // Function to show messages in the status bar with color based on message type
    void showMessageInStatusBar(const QString &message, MessageType type);

    bool writeMediaTags(const QString& filePath, const QPixmap& coverArt);
    QProcess* ffmpegProcess;

    // Pointer to the UI object
    Ui::MainWindow *ui;

    // Member variable for storing the selected movie file path
    QString movieFile;

    // QLabel for status bar message
    QLabel *statusLabel = nullptr;  // New member to hold the QLabel widget

    // TMDb api key
    QString tmdbApiKey;

    // TMDb client
    TmdbClient* tmdbClient;
};

#endif // MAINWINDOW_H
