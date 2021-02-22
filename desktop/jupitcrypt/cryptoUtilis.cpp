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

#include "cryptoUtilis.h"
#include "definevalues.h"

#include <QRandomGenerator>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QTextStream>

constexpr int c_IterationCount = 200000;
constexpr int c_MasterPwdSaltLenght = 32;
constexpr int c_KeyIVLenght = 48;
constexpr int c_KeyLenght = 32;


/**
 * @brief cryptoUtilis::cryptoUtilis
 * @param parent
 */
cryptoUtilis::cryptoUtilis(QObject *parent) : QObject(parent)
{

}

/**
 * @brief cryptoUtilis::getRandomString
 * @param iPwdLenght
 * @return
 */
QString cryptoUtilis::getRandomString(const int &iPwdLenght)
{
    QSettings configFile QSETTINGSCONFIGFILE;
    const QString qstrStdChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    const QString qstrNumbers("0123456789");
    QString qstrChars(qstrStdChars), qstrSpecialChars = configFile.value(PARA_SPECIALCHARS, PARA_DEFAULT_SPECIALCHARS).toString();
    QString randomString;
    int index = 0;

    if(configFile.value(PARA_USENUMBERS, PARA_DEFAULT_USENUMBERS).toBool())
        qstrChars += qstrNumbers;
    if(configFile.value(PARA_USESPECIALCHAR, PARA_DEFAULT_USESPECIALCHAR).toBool())
        qstrChars += qstrSpecialChars;

    // in case QRandomGenerator is predictable we shuffle the inputstring to make it harder to predict
    // (Event though I think QRandomGenerator should be pretty good)
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(qstrChars.begin(), qstrChars.end(), g);
    QRandomGenerator gen = QRandomGenerator::securelySeeded();

    for(int iCount = 0; iCount < iPwdLenght; ++iCount)
    {
        index = gen.generate() % qstrChars.length();
        QChar nextChar = qstrChars.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}

/**
 * @brief cryptoUtilis::decryptPassword
 * @param qstrMasterKey
 * @param salt
 * @param qbaPassword
 * @return
 */
QByteArray cryptoUtilis::decryptPassword(const QString &qstrMasterKey, const unsigned char *salt, const QByteArray &qbaPassword)
{
    EVP_CIPHER_CTX* en = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX* de = EVP_CIPHER_CTX_new();
    QByteArray qbaRet("");
    unsigned char *decrypted;

    if(initAES(qstrMasterKey, salt, en, de) > 0)
    {
        qbaRet = QByteArray::fromHex(qbaPassword);
        int len = qbaRet.length();
        const char* cipherText = qbaRet;

        decrypted = aes_decrypt(de, reinterpret_cast<unsigned char*>(const_cast<char*>(cipherText)), &len);
        qbaRet = QByteArray(reinterpret_cast<char*>(decrypted), len);

        EVP_CIPHER_CTX_free(en);
        EVP_CIPHER_CTX_free(de);
    }
    return qbaRet;
}

/**
 * @brief cryptoUtilis::encryptPassword
 * @param qstrMasterKey
 * @param salt
 * @param qbaPassword
 * @return
 */
QByteArray cryptoUtilis::encryptPassword(const QString &qstrMasterKey, const unsigned char *salt, const QByteArray &qbaPassword)
{
    EVP_CIPHER_CTX* en = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX* de = EVP_CIPHER_CTX_new();
    QByteArray qbaRet("");

    if(initAES(qstrMasterKey, salt, en, de) > 0)
    {
        int len = qbaPassword.length() + 1;
        auto plainText = reinterpret_cast<unsigned char*>(const_cast<char*>(static_cast<const char*>(qbaPassword)));
        unsigned char* cipherText = aes_encrypt(en, plainText, &len);
        qbaRet = QByteArray(reinterpret_cast<char*>(cipherText), len).toHex();

        EVP_CIPHER_CTX_free(en);
        EVP_CIPHER_CTX_free(de);
    }
    return qbaRet;
}

/**
 * @brief cryptoUtilis::initAES
 * @param qstrMasterKey
 * @param salt
 * @param en
 * @param de
 * @return
 */
int cryptoUtilis::initAES(const QString &qstrMasterKey, const unsigned char *salt, EVP_CIPHER_CTX* e_ctx, EVP_CIPHER_CTX* d_ctx)
{
    unsigned char key[c_KeyIVLenght];
    getKeyAndIVwithPBKDF2(qstrMasterKey, salt, c_iSaltLenght, key);

    EVP_CIPHER_CTX_init(e_ctx);
    EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), Q_NULLPTR, key, key + c_KeyLenght);

    EVP_CIPHER_CTX_init(d_ctx);
    EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), Q_NULLPTR, key, key + c_KeyLenght);

    return 1;
}

/**
 * @brief cryptoUtilis::getKeyAndIVwithPBKDF2
 * @param qstrMasterKey
 * @param salt
 * @param keyIV
 * @return
 */
unsigned char * cryptoUtilis::getKeyAndIVwithPBKDF2(const QString &qstrMasterKey, const unsigned char *salt, int iSaltLenght, unsigned char *keyIV)
{
    int key_data_len = static_cast<int>(strlen(reinterpret_cast<char *>(qstrMasterKey.toUtf8().data())));

    if(PKCS5_PBKDF2_HMAC(qstrMasterKey.toUtf8().data(), key_data_len, salt, iSaltLenght, c_IterationCount, EVP_sha512(), c_KeyIVLenght, keyIV) < 1)
        return nullptr;
    return keyIV;
}

/**
 * @brief cryptoUtilis::aes_encrypt
 * @param e
 * @param plaintext
 * @param len
 * @return
 */
unsigned char *cryptoUtilis::aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len)
{
    int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
    unsigned char *ciphertext = reinterpret_cast<unsigned char*>(malloc(static_cast<size_t>(c_len)));

    EVP_EncryptInit_ex(e, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
    EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);
    EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);

    *len = c_len + f_len;
    return ciphertext;
}

/**
 * @brief cryptoUtilis::aes_decrypt
 * @param e
 * @param ciphertext
 * @param len
 * @return
 */
unsigned char *cryptoUtilis::aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len)
{
    int p_len = *len, f_len = 0;
    unsigned char *plaintext = reinterpret_cast<unsigned char*>(malloc(static_cast<size_t>(p_len + AES_BLOCK_SIZE)));

    EVP_DecryptInit_ex(e, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);
    EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
    EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);

    *len = p_len + f_len;
    return plaintext;
}

/**
 * @brief cryptoUtilis::generateSalt
 * @param salt
 * @return
 */
unsigned char *cryptoUtilis::generateSalt(unsigned char *salt, int iSaltLenght)
{
    QRandomGenerator gen = QRandomGenerator::securelySeeded();

    for(int i = 0; i < iSaltLenght; i++)
        salt[i] = static_cast<unsigned char> (gen.generate() % 255);

    return salt;
}

/**
 * @brief cryptoUtilis::checkPassword
 * @param qstrPwd
 * @return
 */
bool cryptoUtilis::checkPassword(const QString &qstrPwd)
{
    QByteArray qbaFilePwdHash;
    QByteArray qbaPwdHashHash;
    unsigned char salt[c_MasterPwdSaltLenght];
    unsigned char hash[c_KeyIVLenght];

    if(qstrPwd.isEmpty())
        return false;

    if(!readPasswordHash(qbaFilePwdHash, salt))
        return false;

    if(qbaFilePwdHash.isEmpty())
    {
        generateSalt(salt, c_MasterPwdSaltLenght);
        qbaPwdHashHash = QByteArray::fromRawData(reinterpret_cast<char *>(getKeyAndIVwithPBKDF2(qstrPwd, salt, c_MasterPwdSaltLenght, hash)), c_KeyIVLenght).toHex();
        savePasswordHash(qbaPwdHashHash, salt);
        return true;
    }

    qbaPwdHashHash = QByteArray::fromRawData(reinterpret_cast<char *>(getKeyAndIVwithPBKDF2(qstrPwd, salt, c_MasterPwdSaltLenght, hash)), c_KeyIVLenght).toHex();
    if(qbaPwdHashHash == qbaFilePwdHash)
        return true;
    return false;
}

/**
 * @brief cryptoUtilis::readPasswordHash
 * @param qbaPwdHash
 * @return
 */
bool cryptoUtilis::readPasswordHash(QByteArray &qbaPwdHash, unsigned char *salt)
{
    QFile file(PWDHASHFILE);

    qbaPwdHash.clear();

    if(!file.exists()) // first start set new password
        return true;

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        QString qstrLine;

        while(!stream.atEnd())
        {
            qstrLine = stream.readLine();

            if(qstrLine.startsWith(QStringLiteral("[ChiperText]=")))
            {
                qstrLine.remove(QStringLiteral("[ChiperText]="));
                qbaPwdHash = qstrLine.toUtf8();
            }
            else if(qstrLine.startsWith(QStringLiteral("[Salt]=")))
            {
                qstrLine.remove(QStringLiteral("[Salt]="));
                salt = saltFromString(qstrLine, salt, c_MasterPwdSaltLenght);
            }
        }
        file.close();
        return true;
    }
    return false;
}

/**
 * @brief cryptoUtilis::savePasswordHash
 * @param qbaPwdHash
 * @return
 */
bool cryptoUtilis::savePasswordHash(const QByteArray &qbaPwdHash, unsigned char *salt)
{
    QFile file(PWDHASHFILE);
    QDir dir(PWDSDIR);

    if(!dir.exists(PWDSDIR))
        dir.mkpath(PWDSDIR);

    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        QString qstrLine;

        stream << QStringLiteral("[ChiperText]=") + qbaPwdHash + "\n";
        stream << QStringLiteral("[Salt]=") + saltToString(salt, c_MasterPwdSaltLenght);

        file.close();
        return true;
    }
    return false;
}

/**
 * @brief cryptoUtilis::saltToString
 * @param salt
 * @param iSaltLenght
 * @return
 */
QString cryptoUtilis::saltToString(unsigned char const *salt, int iSaltLenght)
{
    QString qstrRet;

    for(int i = 0; i < iSaltLenght; i++)
    {
        qstrRet += QString::number(salt[i]);

        if( i != iSaltLenght - 1)
            qstrRet += ",";
    }
    return qstrRet;
}

/**
 * @brief cryptoUtilis::saltFromString
 * @param qstrSalt
 * @param salt
 * @param iSaltLenght
 * @return
 */
unsigned char *cryptoUtilis::saltFromString(const QString &qstrSalt, unsigned char *salt, int iSaltLenght)
{
    QStringList qstrlTemp = qstrSalt.split(",", Qt::SkipEmptyParts);

    if(qstrlTemp.count() == iSaltLenght)
    {
        for(int i = 0; i < iSaltLenght; i++)
            salt[i] = static_cast<unsigned char> (qstrlTemp.at(i).toUInt());
    }
    return salt;
}
