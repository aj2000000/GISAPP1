#ifndef LEFTSIDEBAR_H
#define LEFTSIDEBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QToolButton>
#include <QString>

namespace GISApp::UI {

class LeftSidebar : public QWidget
{
    Q_OBJECT

public:
    explicit LeftSidebar(QWidget *parent = nullptr);
    virtual ~LeftSidebar() = default;

signals:
    void actionTriggered(const QString &name);

private:
    void setupButtons();
    QToolButton *createButton(const QString &name, const QString &text, bool isCheckable);

    QVBoxLayout *m_layout;
    QVBoxLayout *m_topLayout;
    QVBoxLayout *m_bottomLayout;
    QButtonGroup *m_buttonGroup;
};

} // namespace GISApp::UI

#endif // LEFTSIDEBAR_H
