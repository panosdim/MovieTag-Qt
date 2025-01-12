#ifndef MOVIEITEMWIDGET_H
#define MOVIEITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MovieItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieItemWidget(const QString &title, const QString &year,
                             const QString &description, QWidget *parent = nullptr);

    void setCoverImage(const QPixmap &pixmap); // Method to set the cover image
    QPixmap coverImage() const;                // Method to get the cover image

private:
    QLabel *coverLabel;
    QLabel *titleLabel;
    QLabel *yearLabel;
    QLabel *descriptionLabel;
};

#endif // MOVIEITEMWIDGET_H
