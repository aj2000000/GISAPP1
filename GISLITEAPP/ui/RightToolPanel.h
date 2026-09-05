#ifndef RIGHTTOOLPANEL_H
#define RIGHTTOOLPANEL_H

#include <QFrame>
#include <QVBoxLayout>
#include <QToolButton>
#include <QString>

namespace GISApp::UI {

class RightToolPanel : public QFrame
{
    Q_OBJECT

public:
    explicit RightToolPanel(QWidget *parent = nullptr);
    virtual ~RightToolPanel() = default;

signals:
    void toolTriggered(const QString &name);

private:
    QToolButton *createButton(const QString &name, const QString &text);

    QVBoxLayout *m_layout;
};

} // namespace GISApp::UI

#endif // RIGHTTOOLPANEL_H
