#include "backend.h"
#include <QFile>
#include <QDebug>
#include <conio.h>
#include <stdio.h>
#include "Windows.h"
#include "winuser.h"
#include <cmath>
#include <iostream>
#include <iomanip>
#include <complex>
#include <string>
#include <cstring>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <QGuiApplication>
#include <QRandomGenerator>
#include <QClipboard>
#include <QImage>
#include <QPoint>
#include <QPdfWriter>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPicture>



using namespace std;

Backend::Backend(QObject *parent): QObject{parent}
{
    clipboard = QGuiApplication::clipboard();
    timer = new QTimer();
    timer->setSingleShot(true);
}

void Backend::send_sites()  // выводим в окошко сайты
{
    QStringList sites_array;
    QJsonArray jsonArray = json_text["urls"].toArray();

    foreach (const QJsonValue & value, jsonArray)
        sites_array << value.toString();

    emit send_sites_array(sites_array);  // сигнал для отправки в qml
}

void Backend::get_login_pass(bool pass, QString site)  // получение пароля или логина при копировании
{    
    QJsonObject site_lp = json_text[site].toObject();
    QString query;

    if (pass==0)
        query = site_lp["login"].toString();

    if (pass==1)
        query = site_lp["password"].toString();

    unsigned char *encrypt = convert_format(query),
            decrypt[128];

    crypt_data(encrypt, decrypt, 0, key_hash);

    string buffer(reinterpret_cast<char*>(decrypt));

    send_to_buffer(buffer);

    buffer.clear();
    decrypt[0] = '\0';
}

bool Backend::get_pass(QString pass)  // проверка открытия файла хешем
{
    SHA256(reinterpret_cast<unsigned char *>(pass.toLatin1().data()), pass.length(), key_hash);
    pass.clear();  // удаление открытого пароля
    decrypt_file(key_hash);

    if (all_data.substr(0,1) == "{")
    {
        data_to_json();
        return true;
    }
    else
        return false;

}

unsigned char* Backend::convert_format(QString qstring)
{
    unsigned char* unchar;
    strcpy((char*)unchar, qstring.toLatin1().toStdString().c_str());

    return unchar;
}

void Backend::data_to_json()  // создаем док json в оперативке
{
    QString qstring_data = QString::fromStdString(all_data);

    QJsonParseError error;
    QJsonDocument json_document = QJsonDocument::fromJson(qstring_data.toUtf8(), &error);
    if (json_document.isObject())
        json_text = json_document.object();
}

QString Backend::unchar_to_qstr(char* unchar)
{
    return QString::fromLatin1(QByteArray::fromStdString(string(unchar)));
}


QString Backend::get_random_string()
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 10; // пароль длиной 10

    QString randomString;
    for(int i=0; i<randomStringLength; ++i)
    {
        int index =  QRandomGenerator::global()->bounded(0, possibleCharacters.length());
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}

void Backend::get_new_account(QString site, QString login, QString pass) // запись в файл и в форму нового аккаунта
{
    if (login == "") {
        login = get_random_string();
    }

    unsigned char *source_login = reinterpret_cast<unsigned char *>(login.toLatin1().data()),
            cipher_login[128];
    crypt_data(source_login, cipher_login, 1, key_hash);

    if (pass == ""){
        pass = get_random_string();
    }

    unsigned char *source_pass = reinterpret_cast<unsigned char *>(pass.toLatin1().data()),
            cipher_pass[128];
    crypt_data(source_pass, cipher_pass, 1, key_hash);

    QJsonObject json_site = json_text[site].toObject();

    json_site.insert("login", unchar_to_qstr((char*)cipher_login));
    json_site.insert("password", unchar_to_qstr((char*)cipher_pass));

    cipher_login[0] = '\0';
    cipher_pass[0] = '\0';

    json_text.insert(site, json_site);

    QJsonArray jsonArray = json_text["urls"].toArray();
    jsonArray.append(site);
    json_text.insert("urls", jsonArray);

    encrypt_file(key_hash);  // сохраняется файл с обновлением
    send_sites();  // обновление формы
}


void Backend::send_to_buffer(const string &buffer)
{
    clipboard->setText(buffer.c_str());

    timer->stop();
    connect(timer, SIGNAL(timeout()), this, SLOT(ClearClipboard()));
    timer->start(20000);
}


void Backend::ClearClipboard()
{
    clipboard->clear();
    //qDebug() << "Буфер очищен";
}


bool Backend::decrypt_file(unsigned char* key_user){
    all_data = "";

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    unsigned char * iv = (unsigned char *)("%QkI0j2XwDjpf$644yfm0xY0wBsWkM1h"),
            *key = (unsigned char *) (char*) key_user;

    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_ofb(), NULL, key, iv))
        return false;

    QFile source_file(sourcefile);

    source_file.open(QIODevice::ReadOnly);

    unsigned char ciphertext[256];  // расшифрованный текст
    unsigned char plaintext[256];  // исходный шифртекст
    int plaintext_len = source_file.read((char *)plaintext, 256);
    int len;

    while (plaintext_len > 0)
    {
        if(!EVP_DecryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
            return false;
        string all_data_(ciphertext, ciphertext+len);
        all_data += all_data_;

        plaintext_len = source_file.read((char *)plaintext, 256);
    }

    if(!EVP_DecryptFinal_ex(ctx, ciphertext + len, &len))
        return false;

    string all_data_(ciphertext, ciphertext+len);
    all_data += all_data_;
    EVP_CIPHER_CTX_free(ctx);

    cout << all_data;
    source_file.close();
}

bool Backend::SHA256(unsigned char* input, unsigned long key_length, unsigned char* key_hash)
{
    SHA256_CTX context;
    SHA256_Init(&context);
    SHA256_Update(&context, (unsigned char*)input, key_length);
    SHA256_Final(key_hash, &context);

    return true;
}


bool Backend::crypt_data(unsigned char *sourcetext, unsigned char *ciphertext, int do_encrypt, unsigned char* key_user)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    int sourcetext_len = strlen((char *)sourcetext),
            len,
            ciphertext_len;

    unsigned char * iv = (unsigned char *)("%QkI0j2XwDjpf$644yfm0xY0wBsWkM1h"),
            *key = (unsigned char *) (char*) key_user;

    if(!EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, NULL, NULL, do_encrypt))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    if(!EVP_CipherInit_ex(ctx, NULL, NULL, key, iv, do_encrypt))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    if(!EVP_CipherUpdate(ctx, ciphertext, &len, sourcetext, sourcetext_len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    ciphertext_len = len;

    if(!EVP_CipherFinal_ex(ctx, ciphertext + len, &len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    ciphertext[ciphertext_len] = '\0';

    return ciphertext_len;
}


bool Backend::encrypt_file(unsigned char* key_user)
{

    all_data = QJsonDocument(json_text).toJson(QJsonDocument::Compact).toStdString();

    QFile file_modified(sourcefile);
    file_modified.open(QIODevice::ReadWrite | QIODevice::Truncate);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    unsigned char * iv = (unsigned char *)("%QkI0j2XwDjpf$644yfm0xY0wBsWkM1h"),
            *key = (unsigned char *) (char*) key_user;

    if(!EVP_EncryptInit_ex(ctx, EVP_aes_256_ofb(), NULL, key, iv))
        return false;

    unsigned char ciphertext[256];
    unsigned char plaintext[256];

    int sourcetext_len = strlen((char *)all_data.substr(0, 256).c_str()),
            len,
            ciphertext_len;

    while (sourcetext_len > 0)
    {
        strcpy((char*)plaintext, (char*)all_data.substr(0, 256).c_str());

        if(!EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, sourcetext_len))
            return false;

        all_data.erase(0, 256);
        sourcetext_len = strlen((char *)all_data.substr(0, 256).c_str());

        file_modified.write((char*)ciphertext, len);
    }

    ciphertext_len = len;

    if(!EVP_CipherFinal_ex(ctx, ciphertext + len, &len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    ciphertext[ciphertext_len] = '\0';

    file_modified.write((char*)ciphertext, len);

    file_modified.close();
}


void Backend::print_all_data()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, 0);
    if (dialog.exec() == QDialog::Accepted) {
        printer.outputFileName();
        printer.setPageSize(QPageSize(QPageSize::A4));

        QPainter painter(&printer);
        painter.setPen(QPen(Qt::gray, 1));
        painter.drawLine(40, 85, 1140, 85);

        QFont font("Times New Roman", 14);
        painter.setFont(font);

        QString login;
        QString pass;

        painter.drawImage(80, 50, create_image("Ресурс"));
        painter.drawImage(455, 50, create_image("Логин"));
        painter.drawImage(830, 50, create_image("Пароль"));

        QJsonArray jsonArray = json_text["urls"].toArray();
        for (int i=0; i < jsonArray.count(); i++){
            int count = i % 30;
            if (i % 30 == 0 && i != 0){
                printer.newPage();
                painter.drawImage(80, 50, create_image("Ресурс"));
                painter.drawImage(455, 50, create_image("Логин"));
                painter.drawImage(830, 50, create_image("Пароль"));
                painter.drawLine(40, 85, 1140, 85);
            }

            QString site = jsonArray[i].toString();
            QJsonObject object_site = json_text[site].toObject();

            login = object_site["login"].toString();
            unsigned char *cipher_login = convert_format(login),
                    source_login[128];

            crypt_data(cipher_login, source_login, 0, key_hash);

            login = unchar_to_qstr((char*)source_login);
            source_login[0] = '\0';

            pass = object_site["password"].toString();
            unsigned char *cipher_pass = convert_format(pass),
                    source_pass[128];

            crypt_data(cipher_pass, source_pass, 0, key_hash);

            pass = unchar_to_qstr((char*)source_pass);
            source_pass[0] = '\0';

            painter.drawLine(40, (count+2)*50+35, 1140, (count+2)*50+35);

            painter.drawImage(80,(count+2)*50, create_image(site));
            painter.drawImage(455,(count+2)*50, create_image(login));
            painter.drawImage(830,(count+2)*50, create_image(pass));

            login.clear();
            pass.clear();
        }
    }
}

QImage Backend::create_image(QString text) // отрисовка картинок с текстом
{
    QImage image(QSize(200, 20),QImage::Format_RGB32);
    QPainter painter(&image);
    painter.fillRect(QRectF(0,0,200,20),Qt::white);
    painter.setPen(QPen(Qt::black));

    QFont font("Times New Roman", 14);
    painter.setFont(font);

    painter.drawText(QRect(0,0,200,20), text);
    return image;
}
