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


using namespace std;

Backend::Backend(QObject *parent): QObject{parent}
{
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

    decrypt_data(encrypt, decrypt, key_hash);

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

void Backend::send_to_buffer(const string &buffer)
{
    OpenClipboard(0);
    EmptyClipboard();
    HGLOBAL hg=GlobalAlloc(GMEM_MOVEABLE, buffer.size()+1);
    if (!hg){
        CloseClipboard();
        return;
    }
    memcpy(GlobalLock(hg),buffer.c_str(), buffer.size()+1);
    GlobalUnlock(hg);
    SetClipboardData(CF_TEXT,hg);
    CloseClipboard();
    GlobalFree(hg);
}

bool Backend::decrypt_data(unsigned char *sourcetext, unsigned char *ciphertext, unsigned char* key_user)
{ // расшивровка логина/пароля
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    int sourcetext_len = strlen((char *)sourcetext),
            len,
            ciphertext_len;

    unsigned char * iv = (unsigned char *)("%QkI0j2XwDjpf$644yfm0xY0wBsWkM1h"),
            *key = (unsigned char *) (char*) key_user;

    if(!EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, NULL, NULL, 0))
    {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    if(!EVP_CipherInit_ex(ctx, NULL, NULL, key, iv, 0))
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

bool Backend::decrypt_file(unsigned char* key_user){
    all_data = "";

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    unsigned char * iv = (unsigned char *)("%QkI0j2XwDjpf$644yfm0xY0wBsWkM1h"),
            *key = (unsigned char *) (char*) key_user;

    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_ofb(), NULL, key, iv))
        return false;

    QFile source_file("D://Notebook//Labs_Kavallini_181_331//Lab_2//Lab_2//Data.json");
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
