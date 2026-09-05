#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include "MainBasePage.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

class LoginPage : public MainBasePage
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    virtual ~LoginPage();

    QString pageTitle() const override { return tr("Sign In"); }

    void clearInputs();
    void showErrorMessage(const QString &message);
    void clearErrorMessage();

signals:
    void loginSuccess(const QString &username);
    void switchToRegisterRequested();

private slots:
    void onLoginClicked();
    void onGuestLoginClicked();

private:
    void setupUi();
    void setupStyles();

    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QCheckBox *m_rememberCheckBox;
    QLabel *m_errorLabel;
    QPushButton *m_loginButton;
    QPushButton *m_switchToRegisterButton;
    QPushButton *m_guestButton;
};

#endif // LOGINPAGE_H
