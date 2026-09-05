#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include <QString>

class LoginPage;
class RegisterPage;

class AuthWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    virtual ~AuthWindow();

    void showLoginPage();
    void showRegisterPage();
    void byPassLoginforTest();
signals:
    void authenticated(const QString &username);

private slots:
    void onLoginSuccess(const QString &username);
    void onRegisterSuccess(const QString &username);


private:
    void setupUi();


    QStackedWidget *m_stackedWidget;
    LoginPage *m_loginPage;
    RegisterPage *m_registerPage;
};

#endif // AUTHWINDOW_H
