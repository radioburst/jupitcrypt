/*
 * This file is part of jupitcrypt. (https://github.com/radioburst/jupitcrypt)
 * Copyright (c) 2021 Andreas Dorrer.
 *
 * jupitcrypt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * jupitcrypt is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with jupitcrypt. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PWDGENERATER_H
#define PWDGENERATER_H

#include <QObject>

#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/evp.h>

class cryptoUtilis : public QObject
{
    Q_OBJECT
public:
    explicit cryptoUtilis(QObject *parent = Q_NULLPTR);

    static QString getRandomString(const int &iPwdLenght);

    static QByteArray decryptPassword(const QString &qstrMasterKey, const unsigned char *salt, const QByteArray &qbaPassword);
    static QByteArray encryptPassword(const QString &qstrMasterKey, const unsigned char *salt, const QByteArray &qbaPassword);
    static int initAES(const QString &qstrMasterKey, const unsigned char *salt, EVP_CIPHER_CTX* e_ctx, EVP_CIPHER_CTX* d_ctx);
    static unsigned char *generateSalt(unsigned char *salt, int iSaltLenght);
    static QString saltToString(const unsigned char *salt, int iSaltLenght);
    static unsigned char *saltFromString(const QString &qstrSalt, unsigned char *salt, int iSaltLenght);
    static bool checkPassword(const QString &qstrPwd);

private:
    static unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len);
    static unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len);
    static bool readPasswordHash(QByteArray &qbaPwdHash, unsigned char *salt);
    static bool savePasswordHash(const QByteArray &qbaPwdHashHash, unsigned char *salt);
    static unsigned char *getKeyAndIVwithPBKDF2(const QString &qstrMasterKey, const unsigned char *salt, int iSaltLenght, unsigned char *keyIV);

public slots:
};

#endif // PWDGENERATER_H
