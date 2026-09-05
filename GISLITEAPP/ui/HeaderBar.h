#ifndef HEADERBAR_H
#define HEADERBAR_H

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>

namespace GISApp::UI {

class HeaderBar : public QFrame
{
    Q_OBJECT

public:
    explicit HeaderBar(QWidget *parent = nullptr);
    virtual ~HeaderBar() = default;

    void setOperatorInfo(const QString &title, const QString &company, const QString &versionAndRole);

signals:
    void actionTriggered(const QString &actionName);

private:
    void setupUi();
    QPushButton *createActionButton(const QString &text, const QString &tooltip);

    QLabel *m_titleLabel;
};

} // namespace GISApp::UI

#endif // HEADERBAR_H
