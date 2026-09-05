#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include "MainBasePage.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

class RegisterPage : public MainBasePage
{
    Q_OBJECT

public:
    explicit RegisterPage(QWidget *parent = nullptr);
    virtual ~RegisterPage();

    QString pageTitle() const override { return tr("Create Account"); }

    void clearInputs();
    void showErrorMessage(const QString &message);
    void clearErrorMessage();

signals:
    void registerSuccess(const QString &username);
    void switchToLoginRequested();

private slots:
    void onRegisterClicked();

private:
    void setupUi();
    void setupStyles();

    QLineEdit *m_fullNameEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_emailEdit;
    QComboBox *m_roleCombo;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_confirmPasswordEdit;
    QLabel *m_errorLabel;
    QPushButton *m_registerButton;
    QPushButton *m_switchToLoginButton;
};

#endif // REGISTERPAGE_H
