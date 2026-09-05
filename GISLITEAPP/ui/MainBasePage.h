#ifndef MAINBASEPAGE_H
#define MAINBASEPAGE_H

#include <QWidget>
#include <QString>

class MainBasePage : public QWidget
{
    Q_OBJECT

public:
    explicit MainBasePage(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }

    virtual ~MainBasePage() = default;

    virtual QString pageTitle() const = 0;
    virtual void onActivated() {}
    virtual void onDeactivated() {}
};

#endif // MAINBASEPAGE_H
