#include "AuthWindow.h"
#include "LoginPage.h"
#include "RegisterPage.h"

#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QIcon>

AuthWindow::AuthWindow(QWidget *parent)
    : QWidget(parent)
    , m_stackedWidget(nullptr)
    , m_loginPage(nullptr)
    , m_registerPage(nullptr)
{
    setupUi();
}

AuthWindow::~AuthWindow()
{
}

void AuthWindow::setupUi()
{
    setObjectName(QStringLiteral("AuthWindow"));
    setAttribute(Qt::WA_StyledBackground, true);
    setWindowTitle(tr("GISLITE — Authentication"));
    setFixedSize(480, 640);

    // Center on primary screen
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_stackedWidget = new QStackedWidget(this);

    m_loginPage = new LoginPage(this);
    m_registerPage = new RegisterPage(this);

    m_stackedWidget->addWidget(m_loginPage);
    m_stackedWidget->addWidget(m_registerPage);

    layout->addWidget(m_stackedWidget);

    // Page Switching Connections
    connect(m_loginPage, &LoginPage::switchToRegisterRequested, this, &AuthWindow::showRegisterPage);
    connect(m_registerPage, &RegisterPage::switchToLoginRequested, this, &AuthWindow::showLoginPage);

    // Authentication Success Connections
    connect(m_loginPage, &LoginPage::loginSuccess, this, &AuthWindow::onLoginSuccess);
    connect(m_registerPage, &RegisterPage::registerSuccess, this, &AuthWindow::onRegisterSuccess);

    m_stackedWidget->setCurrentWidget(m_loginPage);

}

void AuthWindow::byPassLoginforTest()
{
    emit authenticated("aman");
}

void AuthWindow::showLoginPage()
{
    m_loginPage->clearInputs();
    m_stackedWidget->setCurrentWidget(m_loginPage);
}

void AuthWindow::showRegisterPage()
{
    m_registerPage->clearInputs();
    m_stackedWidget->setCurrentWidget(m_registerPage);
}

void AuthWindow::onLoginSuccess(const QString &username)
{
    emit authenticated(username);
}

void AuthWindow::onRegisterSuccess(const QString &username)
{
    emit authenticated(username);
}
