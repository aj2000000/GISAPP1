/**
 * @file Aes256Cipher.cpp
 * @brief Implementation of OpenSSL EVP AES-256-CBC cipher routines.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "Aes256Cipher.h"
#include <QDebug>

namespace GISApp::Communication::Udp::Security {

Aes256Cipher::Aes256Cipher()
{
    // Default 32-byte key and 16-byte IV
    m_key = QByteArray::fromHex("001122334455667fcd99aabbccddeeff00112233445566778899aa78aafb5c");
    m_iv  = QByteArray::fromHex("e90203040aab0708090a0b0c0d0e0f10");
}

Aes256Cipher::Aes256Cipher(const QByteArray &key, const QByteArray &iv)
    : m_key(key), m_iv(iv)
{
}

QByteArray Aes256Cipher::encrypt(const QByteArray &plainText)
{
    if (plainText.isEmpty()) {
        return QByteArray();
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "[Aes256Cipher] Failed to allocate EVP_CIPHER_CTX for encryption.";
        return QByteArray();
    }

    QByteArray encrypted;
    encrypted.resize(plainText.size() + AES_BLOCK_SIZE);

    int len = 0;
    int ciphertextLen = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(m_key.constData()),
                           reinterpret_cast<const unsigned char*>(m_iv.constData())) != 1) {
        qWarning() << "[Aes256Cipher] EVP_EncryptInit_ex failed.";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(encrypted.data()), &len,
                          reinterpret_cast<const unsigned char*>(plainText.constData()), plainText.size()) != 1) {
        qWarning() << "[Aes256Cipher] EVP_EncryptUpdate failed.";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    ciphertextLen = len;

    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encrypted.data()) + len, &len) != 1) {
        qWarning() << "[Aes256Cipher] EVP_EncryptFinal_ex failed.";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    ciphertextLen += len;

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(ciphertextLen);
    return encrypted;
}

QByteArray Aes256Cipher::decrypt(const QByteArray &cipherText)
{
    if (cipherText.isEmpty()) {
        return QByteArray();
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "[Aes256Cipher] Failed to allocate EVP_CIPHER_CTX for decryption.";
        return QByteArray();
    }

    QByteArray decrypted;
    decrypted.resize(cipherText.size());

    int len = 0;
    int plaintextLen = 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(m_key.constData()),
                           reinterpret_cast<const unsigned char*>(m_iv.constData())) != 1) {
        qWarning() << "[Aes256Cipher] EVP_DecryptInit_ex failed.";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decrypted.data()), &len,
                          reinterpret_cast<const unsigned char*>(cipherText.constData()), cipherText.size()) != 1) {
        qWarning() << "[Aes256Cipher] EVP_DecryptUpdate failed.";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    plaintextLen = len;

    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decrypted.data()) + len, &len) != 1) {
        qWarning() << "[Aes256Cipher] EVP_DecryptFinal_ex failed (bad key/IV or corrupted ciphertext).";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    plaintextLen += len;

    EVP_CIPHER_CTX_free(ctx);
    decrypted.resize(plaintextLen);
    return decrypted;
}

} // namespace GISApp::Communication::Udp::Security
