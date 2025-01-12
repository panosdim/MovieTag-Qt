#include "movieitemwidget.h"

MovieItemWidget::MovieItemWidget(const QString &title, const QString &year,
                                 const QString &description, QWidget *parent)
    : QWidget(parent)
{
    // Initialize the QLabel for the cover
    coverLabel = new QLabel(this);
    coverLabel->setFixedSize(100, 150);
    coverLabel->setText("Loading..."); // Placeholder text while image loads

    // Title with Styling
    titleLabel = new QLabel(this);
    titleLabel->setText(QString("<span style='font-size:24px; font-weight:bold;'>%1</span>").arg(title));
    titleLabel->setWordWrap(true);

    // Release Year with Styling
    yearLabel = new QLabel(this);
    yearLabel->setText(QString("<span style='font-size:18px; font-weight:bold;'>%1</span>").arg(year));
    yearLabel->setWordWrap(false);

    // Description
    descriptionLabel = new QLabel(description, this);
    descriptionLabel->setWordWrap(true);

    // Layouts
    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(yearLabel);
    textLayout->addWidget(descriptionLabel);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(coverLabel);
    mainLayout->addLayout(textLayout);
    setLayout(mainLayout);
}

void MovieItemWidget::setCoverImage(const QPixmap &pixmap)
{
    coverLabel->setPixmap(pixmap.scaled(100, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QPixmap MovieItemWidget::coverImage() const
{
    return coverLabel->pixmap(Qt::ReturnByValue);  // Get the QPixmap directly
}



