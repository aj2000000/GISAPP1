#include "RegisterPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QAction>
#include <QStyle>
#include <QRegularExpression>

RegisterPage::RegisterPage(QWidget *parent)
    : MainBasePage(parent)
    , m_fullNameEdit(nullptr)
    , m_usernameEdit(nullptr)
    , m_emailEdit(nullptr)
    , m_roleCombo(nullptr)
    , m_passwordEdit(nullptr)
    , m_confirmPasswordEdit(nullptr)
    , m_errorLabel(nullptr)
    , m_registerButton(nullptr)
    , m_switchToLoginButton(nullptr)
{
    setupUi();
    setupStyles();
}

RegisterPage::~RegisterPage()
{
}

void RegisterPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(36, 32, 36, 32);
    mainLayout->setSpacing(12);

    // Header
    QLabel *title = new QLabel(tr("Create Account"), this);
    title->setObjectName("registerTitle");
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel(tr("Set up your profile to access GISLITE"), this);
    subtitle->setObjectName("registerSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(4);

    // Error Label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName("registerErrorLabel");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    // Inputs
    QLabel *nameLabel = new QLabel(tr("Full Name"), this);
    nameLabel->setObjectName("fieldLabel");
    m_fullNameEdit = new QLineEdit(this);
    m_fullNameEdit->setPlaceholderText(tr("e.g. John Doe"));

    QLabel *userLabel = new QLabel(tr("Username"), this);
    userLabel->setObjectName("fieldLabel");
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(tr("Choose a username"));

    QLabel *emailLabel = new QLabel(tr("Email Address"), this);
    emailLabel->setObjectName("fieldLabel");
    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText(tr("name@example.com"));

    QLabel *roleLabel = new QLabel(tr("Role / Responsibility"), this);
    roleLabel->setObjectName("fieldLabel");
    m_roleCombo = new QComboBox(this);
    m_roleCombo->addItems({tr("GIS Analyst"), tr("Operator / Field Officer"), tr("Administrator"), tr("Viewer")});

    QLabel *passLabel = new QLabel(tr("Password"), this);
    passLabel->setObjectName("fieldLabel");
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText(tr("At least 6 characters"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    QLabel *confirmPassLabel = new QLabel(tr("Confirm Password"), this);
    confirmPassLabel->setObjectName("fieldLabel");
    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setPlaceholderText(tr("Re-enter password"));
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(m_fullNameEdit);
    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(m_usernameEdit);
    mainLayout->addWidget(emailLabel);
    mainLayout->addWidget(m_emailEdit);
    mainLayout->addWidget(roleLabel);
    mainLayout->addWidget(m_roleCombo);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(m_passwordEdit);
    mainLayout->addWidget(confirmPassLabel);
    mainLayout->addWidget(m_confirmPasswordEdit);

    mainLayout->addSpacing(8);

    // Register Button
    m_registerButton = new QPushButton(tr("Register & Continue"), this);
    m_registerButton->setObjectName("primaryButton");
    m_registerButton->setCursor(Qt::PointingHandCursor);
    m_registerButton->setDefault(true);
    mainLayout->addWidget(m_registerButton);

    mainLayout->addSpacing(4);

    // Switch to Login Link
    QHBoxLayout *loginLinkLayout = new QHBoxLayout();
    QLabel *haveAccountLabel = new QLabel(tr("Already registered?"), this);
    haveAccountLabel->setStyleSheet("color: #718096; font-size: 13px;");

    m_switchToLoginButton = new QPushButton(tr("Sign In"), this);
    m_switchToLoginButton->setObjectName("linkButton");
    m_switchToLoginButton->setCursor(Qt::PointingHandCursor);
    m_switchToLoginButton->setFlat(true);

    loginLinkLayout->addStretch();
    loginLinkLayout->addWidget(haveAccountLabel);
    loginLinkLayout->addWidget(m_switchToLoginButton);
    loginLinkLayout->addStretch();
    mainLayout->addLayout(loginLinkLayout);

    mainLayout->addStretch();

    // Connections
    connect(m_registerButton, &QPushButton::clicked, this, &RegisterPage::onRegisterClicked);
    connect(m_confirmPasswordEdit, &QLineEdit::returnPressed, this, &RegisterPage::onRegisterClicked);
    connect(m_switchToLoginButton, &QPushButton::clicked, this, &RegisterPage::switchToLoginRequested);
}

void RegisterPage::setupStyles()
{
    setStyleSheet(
        "QWidget { font-family: 'Segoe UI', Arial, sans-serif; }"
        "QLabel#registerTitle { font-size: 24px; font-weight: 700; color: #2B6CB0; }"
        "QLabel#registerSubtitle { font-size: 13px; color: #718096; margin-bottom: 4px; }"
        "QLabel#fieldLabel { font-size: 12px; font-weight: 600; color: #4A5568; margin-top: 2px; }"
        "QLabel#registerErrorLabel { background-color: #FED7D7; color: #9B2C2C; border-radius: 6px; padding: 6px; font-size: 12px; }"
        "QLineEdit, QComboBox { border: 1px solid #CBD5E0; border-radius: 6px; padding: 7px 10px; font-size: 13px; background-color: #FFFFFF; color: #2D3748; }"
        "QLineEdit:focus, QComboBox:focus { border: 1px solid #3182CE; }"
        "QPushButton#primaryButton { background-color: #2B6CB0; color: #FFFFFF; font-size: 14px; font-weight: 600; padding: 10px; border-radius: 6px; border: none; }"
        "QPushButton#primaryButton:hover { background-color: #2C5282; }"
        "QPushButton#primaryButton:pressed { background-color: #1A365D; }"
        "QPushButton#linkButton { color: #3182CE; font-size: 13px; font-weight: 600; border: none; padding: 0; }"
        "QPushButton#linkButton:hover { color: #2B6CB0; text-decoration: underline; }"
    );
}

void RegisterPage::onRegisterClicked()
{
    QString fullName = m_fullNameEdit->text().trimmed();
    QString username = m_usernameEdit->text().trimmed();
    QString email = m_emailEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();

    if (username.isEmpty()) {
        showErrorMessage(tr("Username is required."));
        m_usernameEdit->setFocus();
        return;
    }

    if (email.isEmpty() || !email.contains(QRegularExpression(R"([^@]+@[^@]+\.[^@]+)"))) {
        showErrorMessage(tr("Please provide a valid email address."));
        m_emailEdit->setFocus();
        return;
    }

    if (password.length() < 6) {
        showErrorMessage(tr("Password must be at least 6 characters long."));
        m_passwordEdit->setFocus();
        return;
    }

    if (password != confirmPassword) {
        showErrorMessage(tr("Passwords do not match."));
        m_confirmPasswordEdit->setFocus();
        return;
    }

    clearErrorMessage();
    emit registerSuccess(username);
}

void RegisterPage::clearInputs()
{
    m_fullNameEdit->clear();
    m_usernameEdit->clear();
    m_emailEdit->clear();
    m_passwordEdit->clear();
    m_confirmPasswordEdit->clear();
    clearErrorMessage();
}

void RegisterPage::showErrorMessage(const QString &message)
{
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

void RegisterPage::clearErrorMessage()
{
    m_errorLabel->clear();
    m_errorLabel->hide();
}
