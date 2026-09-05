#include "LoginPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QAction>
#include <QIcon>
#include <QStyle>
#include <QGraphicsDropShadowEffect>

LoginPage::LoginPage(QWidget *parent)
    : MainBasePage(parent)
    , m_usernameEdit(nullptr)
    , m_passwordEdit(nullptr)
    , m_rememberCheckBox(nullptr)
    , m_errorLabel(nullptr)
    , m_loginButton(nullptr)
    , m_switchToRegisterButton(nullptr)
    , m_guestButton(nullptr)
{
    setupUi();
    setupStyles();
}

LoginPage::~LoginPage()
{
}

void LoginPage::setupUi()
{
    setObjectName(QStringLiteral("LoginPage"));
    setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(36, 40, 36, 40);
    mainLayout->setSpacing(16);

    // Header / Brand
    QLabel *brandTitle = new QLabel(tr("GISLITE"), this);
    brandTitle->setObjectName("loginBrandTitle");
    brandTitle->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel(tr("Sign in to your GIS workspace"), this);
    subtitle->setObjectName("loginSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(brandTitle);
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(8);

    // Error Label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName("loginErrorLabel");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    // Input Fields
    QLabel *userLabel = new QLabel(tr("Username / Email"), this);
    userLabel->setObjectName("fieldLabel");
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(tr("Enter your username"));
    m_usernameEdit->setClearButtonEnabled(true);

    QLabel *passLabel = new QLabel(tr("Password"), this);
    passLabel->setObjectName("fieldLabel");
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText(tr("Enter your password"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    // Toggle password visibility action
    QAction *togglePassAction = m_passwordEdit->addAction(
        style()->standardIcon(QStyle::SP_FileDialogContentsView),
        QLineEdit::TrailingPosition);
    connect(togglePassAction, &QAction::triggered, this, [this, togglePassAction]() {
        if (m_passwordEdit->echoMode() == QLineEdit::Password) {
            m_passwordEdit->setEchoMode(QLineEdit::Normal);
        } else {
            m_passwordEdit->setEchoMode(QLineEdit::Password);
        }
    });

    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(m_usernameEdit);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(m_passwordEdit);

    // Remember me row
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    m_rememberCheckBox = new QCheckBox(tr("Remember me"), this);
    m_rememberCheckBox->setChecked(true);
    optionsLayout->addWidget(m_rememberCheckBox);
    optionsLayout->addStretch();
    mainLayout->addLayout(optionsLayout);

    mainLayout->addSpacing(8);

    // Action Buttons
    m_loginButton = new QPushButton(tr("Sign In"), this);
    m_loginButton->setObjectName("primaryButton");
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setDefault(true);
    mainLayout->addWidget(m_loginButton);

    m_guestButton = new QPushButton(tr("Continue as Guest / Offline"), this);
    m_guestButton->setObjectName("secondaryButton");
    m_guestButton->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(m_guestButton);

    mainLayout->addSpacing(8);

    // Switch to Register Link
    QHBoxLayout *registerLinkLayout = new QHBoxLayout();
    QLabel *noAccountLabel = new QLabel(tr("Don't have an account?"), this);
    noAccountLabel->setObjectName(QStringLiteral("mutedNoticeLabel"));

    m_switchToRegisterButton = new QPushButton(tr("Register here"), this);
    m_switchToRegisterButton->setObjectName("linkButton");
    m_switchToRegisterButton->setCursor(Qt::PointingHandCursor);
    m_switchToRegisterButton->setFlat(true);

    registerLinkLayout->addStretch();
    registerLinkLayout->addWidget(noAccountLabel);
    registerLinkLayout->addWidget(m_switchToRegisterButton);
    registerLinkLayout->addStretch();
    mainLayout->addLayout(registerLinkLayout);

    mainLayout->addStretch();

    // Connections
    connect(m_loginButton, &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);
    connect(m_guestButton, &QPushButton::clicked, this, &LoginPage::onGuestLoginClicked);
    connect(m_switchToRegisterButton, &QPushButton::clicked, this, &LoginPage::switchToRegisterRequested);
}

void LoginPage::setupStyles()
{
    // Styling is centralized and managed by ThemeManager
}

void LoginPage::onLoginClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (username.isEmpty()) {
        showErrorMessage(tr("Please enter your username or email."));
        m_usernameEdit->setFocus();
        return;
    }

    if (password.isEmpty()) {
        showErrorMessage(tr("Please enter your password."));
        m_passwordEdit->setFocus();
        return;
    }

    clearErrorMessage();
    emit loginSuccess(username);
}

void LoginPage::onGuestLoginClicked()
{
    clearErrorMessage();
    emit loginSuccess(tr("Guest"));
}

void LoginPage::clearInputs()
{
    m_usernameEdit->clear();
    m_passwordEdit->clear();
    clearErrorMessage();
}

void LoginPage::showErrorMessage(const QString &message)
{
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

void LoginPage::clearErrorMessage()
{
    m_errorLabel->clear();
    m_errorLabel->hide();
}
