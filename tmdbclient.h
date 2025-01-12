#ifndef TMDBCLIENT_H
#define TMDBCLIENT_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>

class TmdbClient : public QObject
{
    Q_OBJECT

public:
    enum class ErrorSource {
        Configuration,
        Search,
        PosterDownload
    };
    Q_ENUM(ErrorSource)

    explicit TmdbClient(const QString& bearerToken, QObject *parent = nullptr);
    ~TmdbClient();

    void getConfiguration();
    void searchMovie(const QString& query);
    void downloadMoviePoster(const QString& posterPath, QObject *sender);  // Updated method signature

signals:
    void error(ErrorSource source, const QString& message);
    void searchCompleted(const QJsonArray& movies);
    void posterDownloaded(const QByteArray& imageData, const QString& posterPath);  // Updated signal
    void configurationComplete();

private slots:
    void handleConfigurationResponse(QNetworkReply* reply);
    void handleSearchResponse(QNetworkReply* reply);
    void handlePosterDownload(QNetworkReply* reply, QObject *sender, const QString& posterPath);  // Updated method signature

private:
    QString m_bearerToken;
    QString m_baseUrl;
    QString m_posterSize;
    QNetworkAccessManager* m_networkManager;
    bool m_isConfigured;

    static const QString API_BASE_URL;
    QNetworkRequest createRequest(const QString& endpoint) const;
};

#endif // TMDBCLIENT_H
