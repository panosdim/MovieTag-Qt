#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "movieitemwidget.h"
#include "mediatagwriter.h"
#include <QFileDialog>
#include <QString>
#include <QFile>
#include <QLabel>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tmdbClient(nullptr)
{
    ui->setupUi(this);

    ui->btnSearch->setStyleSheet(
        "QPushButton {"
        "    background-color: lightblue;"
        "    color: black;"
        "}"
        "QPushButton:disabled {"
        "    background-color: lightgray;"  // Change background when disabled
        "    color: gray;"  // Change text color when disabled
        "}"
        );

    ui->btnWriteTags->setStyleSheet(
        "QPushButton {"
        "    background-color: lightgreen;"
        "    color: black;"
        "}"
        "QPushButton:disabled {"
        "    background-color: lightgray;"  // Change background when disabled
        "    color: gray;"  // Change text color when disabled
        "}"
        );

    // Initial status message
    showMessageInStatusBar("Please select a movie file by clicking on 'Open Movie'", MessageType::Info);

    // Call readConfigFile to initialize settings
    readConfigFile();

    // Create the client with your API key
    tmdbClient = new TmdbClient(tmdbApiKey, this);

    connect(tmdbClient, &TmdbClient::configurationComplete,
            this, []() {
                qDebug() << "Configuration completed successfully";
            });

    // Connect to error signal
    connect(tmdbClient, &TmdbClient::error,
            this, [this](TmdbClient::ErrorSource source, const QString& message) {
                QString sourceStr;
                switch (source) {
                case TmdbClient::ErrorSource::Configuration:
                    sourceStr = "Configuration";
                    this->ui->btnOpenMovie->setDisabled(true);
                    this->showMessageInStatusBar("Couldn't get configuration check your API key in config.ini", MessageType::Error);
                    break;
                case TmdbClient::ErrorSource::Search:
                    sourceStr = "Search";
                    break;
                case TmdbClient::ErrorSource::PosterDownload:
                    sourceStr = "Poster Download";
                    break;
                }
                qDebug() << "TMDB Error in" << sourceStr << ":" << message;
            });

    // Get configuration
    tmdbClient->getConfiguration();

    // Connect the signal for search
    connect(tmdbClient, &TmdbClient::searchCompleted,
            this, &MainWindow::onSearchCompleted);

    // Connect button signals to slot
    connect(ui->btnOpenMovie, &QPushButton::clicked, this, &MainWindow::onOpenMovieButtonClick);
    connect(ui->btnSearch, &QPushButton::clicked, this, &MainWindow::onSearchButtonClick);
    connect(ui->btnWriteTags, &QPushButton::clicked, this, &MainWindow::onWriteTagsButtonClick);
    connect(ui->searchResults, &QListWidget::itemSelectionChanged,
           this, &MainWindow::onSearchResultSelectionChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readConfigFile()
{
    QString configFilePath = "config.ini";

    // Check if the file exists
    if (!QFile::exists(configFilePath)) {
        showMessageInStatusBar("Error: config.ini file not found!", MessageType::Error);
        return;
    }

    // If the file exists, attempt to read it
    QSettings settings(configFilePath, QSettings::IniFormat);

    // Read TMDb API key
    tmdbApiKey = settings.value("Settings/tmdb_api_key").toString();

    // Check if the key is valid
    if (tmdbApiKey.isEmpty()) {
        showMessageInStatusBar("Error: Failed to read TMDb API key from config.ini!", MessageType::Error);
        return;
    }

    qDebug() << "TMDb API Key readed successfully";
}

void MainWindow::showMessageInStatusBar(const QString &message, MessageType type)
{
    // Remove previous status message if any
    if (statusLabel) {
        statusBar()->removeWidget(statusLabel);  // Remove the previous widget
        delete statusLabel;  // Delete the old label
    }

    // Create a new QLabel to display the message
    statusLabel = new QLabel(message);

    // Set the color based on the message type and theme
    QString color;
    QPalette palette = QApplication::palette();
    QColor windowColor = palette.color(QPalette::Window);
    QColor textColor = palette.color(QPalette::WindowText);

    // Determine if the theme is dark or light
    bool isDarkTheme = (windowColor.value() < 128); // Brightness threshold (0-255 scale)

    switch (type) {
    case MessageType::Error:
        color = isDarkTheme ? "lightcoral" : "red"; // Light coral for dark theme, red for light theme
        break;
    case MessageType::Warning:
        color = isDarkTheme ? "gold" : "darkorange"; // Gold for dark theme, dark orange for light theme
        break;
    case MessageType::Info:
        color = textColor.name(); // Use the text color from the theme
        break;
    }

    // Apply the color to the label's font
    statusLabel->setStyleSheet(QString("color: %1;").arg(color));

    // Add the new message widget to the status bar
    statusBar()->addWidget(statusLabel);
}

void MainWindow::onOpenMovieButtonClick()
{
    // Get the user's home directory and the default "Videos" folder
    QString videoFolder = QDir::homePath() + "/Videos";

    // Open the file dialog starting from the "Videos" folder
    movieFile = QFileDialog::getOpenFileName(nullptr, "Open Movie File", videoFolder, "Movies (*.mp4 *.mkv)");
    if (movieFile.isEmpty()) return;

    // Clear the selection in the list widget
    ui->searchResults->clearSelection();
    ui->searchResults->clear();

    // Enable or disable the buttons and text fields
    ui->movieSearch->setEnabled(true);
    ui->btnSearch->setEnabled(true);
    ui->btnWriteTags->setEnabled(false);

    // Set the initial search text (filename without extension)
    QString searchText = QFileInfo(movieFile).baseName();

    // Regular expression pattern for movie name extraction
    QRegularExpression movieRegex("([ .\\w']+?)(\\W\\d{4}\\W?.*)");
    QRegularExpressionMatch match = movieRegex.match(movieFile);

    if (match.hasMatch()) {
        // Extract movie name and format it
        QString movieName = match.captured(1).replace(".", " ");
        QStringList words = movieName.split("\\s+");
        for (int i = 0; i < words.size(); ++i) {
            words[i] = words[i].at(0).toUpper() + words[i].mid(1);
        }
        movieName = words.join(" ");
        searchText = movieName;
    }

    // Set the search text in the text field
    ui->movieSearch->setText(searchText);

    // Display the status message
    showMessageInStatusBar("Please press 'Search' button", MessageType::Info);
}

void MainWindow::onSearchButtonClick()
{
    // Display a status message to instruct the user
    showMessageInStatusBar("Please select a movie from the list and press 'Write Tags' button", MessageType::Info);

    // Clear the selection and disable the 'Write Tags' button initially
    ui->searchResults->clearSelection();
    ui->btnWriteTags->setDisabled(true);

    // Use the TMDB Client to search for movies
    QString query = ui->movieSearch->text().trimmed();
    if (query.isEmpty()) {
        QMessageBox::warning(this, "Search Error", "Search query cannot be empty.");
        return;
    }

    tmdbClient->searchMovie(query);
}

void MainWindow::onSearchCompleted(const QJsonArray &results)
{
    // Clear previous results in the QListWidget
    ui->searchResults->clear();

    // Disconnect any existing connections to avoid duplicate slots
    disconnect(tmdbClient, &TmdbClient::posterDownloaded, nullptr, nullptr);

    if (results.isEmpty()) {
        // No results found - update the status bar
        showMessageInStatusBar("No movies found for the search criteria", MessageType::Warning);
        return;
    }

    for (const QJsonValue &resultValue : results) {
        QJsonObject result = resultValue.toObject();

        QString title = result["title"].toString();
        QString year = result["release_date"].toString().left(4);
        QString description = result["overview"].toString();
        QString posterPath = result["poster_path"].toString();

        // Create the custom movie item widget
        MovieItemWidget *itemWidget = new MovieItemWidget(title, year, description);

        // Create a QListWidgetItem and set its associated widget
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(itemWidget->sizeHint());
        ui->searchResults->addItem(item);
        ui->searchResults->setItemWidget(item, itemWidget);

        // Download the poster image if the path is valid
        if (!posterPath.isEmpty() && posterPath.startsWith("/")) {
            // Connect the posterDownloaded signal for this specific poster path
            connect(tmdbClient, &TmdbClient::posterDownloaded, this,
                    [itemWidget, posterPath](const QByteArray &imageData, const QString &movieCoverPath) {
                        if (movieCoverPath == posterPath) {
                            if (imageData.isEmpty()) {
                                qDebug() << "Failed to download image:" << posterPath;
                                itemWidget->setCoverImage(QPixmap(":images/no-cover.png")); // Optionally set a default image
                                return;
                            }

                            QPixmap pixmap;
                            if (pixmap.loadFromData(imageData)) {
                                itemWidget->setCoverImage(pixmap);
                            } else {
                                qDebug() << "Failed to load pixmap";
                                itemWidget->setCoverImage(QPixmap(":images/no-cover.png")); // Optionally set a default image
                            }
                        }
                    });

            // Initiate the poster download
            tmdbClient->downloadMoviePoster(posterPath, this);
        } else {
            qDebug() << "Poster path not found";
            itemWidget->setCoverImage(QPixmap(":images/no-cover.png")); // Optionally set a default image
        }
    }
}

void MainWindow::onSearchResultSelectionChanged()
{
    // Enable the Write Tags button only if an item is selected
    ui->btnWriteTags->setEnabled(!ui->searchResults->selectedItems().isEmpty());
}

void MainWindow::onWriteTagsButtonClick()
{
    QListWidgetItem* selectedItem = ui->searchResults->selectedItems().first();
    MovieItemWidget* movieWidget = qobject_cast<MovieItemWidget*>(
        ui->searchResults->itemWidget(selectedItem)
        );

    if (movieWidget) {
        MediaTagWriter* tagWriter = new MediaTagWriter(this);

        connect(tagWriter, &MediaTagWriter::progressUpdate, this,
                [this](const QString& message) {
                    showMessageInStatusBar(message, MessageType::Info);
                });

        connect(tagWriter, &MediaTagWriter::error, this,
                [this](const QString& message) {
                    showMessageInStatusBar(message, MessageType::Error);
                });

        connect(tagWriter, &MediaTagWriter::success, this,
                [this](const QString& message) {
                    showMessageInStatusBar(message, MessageType::Info);
                });

        tagWriter->writeTagsToFile(movieFile, movieWidget->coverImage());
    }
}
