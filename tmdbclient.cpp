#include "tmdbclient.h"
#include <QUrlQuery>

const QString TmdbClient::API_BASE_URL = "https://api.themoviedb.org/3";

TmdbClient::TmdbClient(const QString& bearerToken, QObject *parent)
    : QObject(parent)
    , m_bearerToken(bearerToken)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_isConfigured(false)
{
}

TmdbClient::~TmdbClient()
{
}

QNetworkRequest TmdbClient::createRequest(const QString& endpoint) const
{
    QUrl url(API_BASE_URL + endpoint);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("accept", "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_bearerToken).toUtf8());

    return request;
}

void TmdbClient::getConfiguration()
{
    QNetworkRequest request = createRequest("/configuration");
    QNetworkReply* reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleConfigurationResponse(reply);
        reply->deleteLater();
    });
}

void TmdbClient::handleConfigurationResponse(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(ErrorSource::Configuration,
                   QString("Network error during configuration: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull()) {
        emit error(ErrorSource::Configuration,
                   "Invalid JSON response during configuration");
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("images")) {
        emit error(ErrorSource::Configuration,
                   "Missing 'images' section in configuration response");
        return;
    }

    QJsonObject images = root["images"].toObject();

    if (!images.contains("secure_base_url") || !images.contains("poster_sizes")) {
        emit error(ErrorSource::Configuration,
                   "Missing required fields in configuration response");
        return;
    }

    // Store secure base URL
    m_baseUrl = images["secure_base_url"].toString();

    // Get poster sizes and select appropriate size
    QJsonArray posterSizes = images["poster_sizes"].toArray();
    m_posterSize = "w500";

    bool hasW500 = false;
    for (const QJsonValue& size : posterSizes) {
        if (size.toString() == "w500") {
            hasW500 = true;
            break;
        }
    }

    if (!hasW500 && !posterSizes.isEmpty()) {
        m_posterSize = posterSizes.first().toString();
    }

    m_isConfigured = true;
    emit configurationComplete();
}

void TmdbClient::searchMovie(const QString& query)
{
    if (query.trimmed().isEmpty()) {
        emit error(ErrorSource::Search, "Search query cannot be empty");
        return;
    }

    QNetworkRequest request = createRequest("/search/movie");
    QUrl url = request.url();
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("query", query);
    url.setQuery(urlQuery);
    request.setUrl(url);

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleSearchResponse(reply);
        reply->deleteLater();
    });
}

void TmdbClient::handleSearchResponse(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(ErrorSource::Search,
                   QString("Network error during search: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull()) {
        emit error(ErrorSource::Search,
                   "Invalid JSON response during search");
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("results")) {
        emit error(ErrorSource::Search,
                   "Missing 'results' field in search response");
        return;
    }

    QJsonArray results = root["results"].toArray();
    emit searchCompleted(results);
}

void TmdbClient::downloadMoviePoster(const QString& posterPath, QObject *sender)
{
    if (!m_isConfigured) {
        emit error(ErrorSource::PosterDownload,
                   "TMDB client not configured. Call getConfiguration first.");
        return;
    }

    if (posterPath.isEmpty()) {
        emit error(ErrorSource::PosterDownload,
                   "Poster path cannot be empty");
        return;
    }

    QString fullUrl = m_baseUrl + m_posterSize + posterPath;
    QUrl url(fullUrl);

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_bearerToken).toUtf8());

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply, sender, posterPath]() {
        handlePosterDownload(reply, sender, posterPath);
        reply->deleteLater();
    });
}

void TmdbClient::handlePosterDownload(QNetworkReply* reply, QObject *sender, const QString& posterPath)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(ErrorSource::PosterDownload,
                   QString("Network error during poster download: %1").arg(reply->errorString()));
        return;
    }

    QByteArray imageData = reply->readAll();
    if (imageData.isEmpty()) {
        emit error(ErrorSource::PosterDownload,
                   "Received empty image data");
        return;
    }

    // Emit the signal with the image data and the poster path
    emit posterDownloaded(imageData, posterPath);
}
