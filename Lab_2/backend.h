#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <openssl/sha.h>
#include <QTimer>
#include <QClipboard>

using namespace std;

class Backend : public QObject
{
    Q_OBJECT


public:
    explicit Backend(QObject *parent = nullptr);
    void send_to_buffer(const string &buffer);
    //bool decrypt_data(unsigned char *sourcetext, unsigned char *ciphertext, unsigned char* key_user);
    bool SHA256(unsigned char* input, unsigned long key_length, unsigned char* key_hash);
    bool decrypt_file(unsigned char* key_user);
    unsigned char* convert_format(QString qstring);
    void data_to_json();
    QString unchar_to_qstr(char* unchar);
    bool crypt_data(unsigned char *sourcetext, unsigned char *ciphertext, int do_encrypt, unsigned char* key_user);
    QString get_random_string();
    bool encrypt_file(unsigned char* key_user);
    QImage create_image(QString text);


private:
    QJsonObject json_text;
    string all_data;
    unsigned char key_hash[SHA256_DIGEST_LENGTH];
    QString sourcefile ="D://Notebook//Labs_Kavallini_181_331//Lab_2//Lab_2//Data.json";
    QTimer *timer;
    QClipboard *clipboard;


signals:
    void send_sites_array(QStringList count);

public slots:
    void send_sites();
    void get_login_pass(bool pass, QString site);
    bool get_pass(QString pass);
    void get_new_account(QString site, QString login, QString pass);
    void print_all_data();
    void ClearClipboard();


};

#endif // BACKEND_H
