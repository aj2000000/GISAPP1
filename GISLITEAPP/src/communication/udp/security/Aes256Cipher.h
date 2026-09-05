/**
 * @file Aes256Cipher.h
 * @brief OpenSSL EVP AES-256-CBC encryption and decryption helper.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 *
 * @class Aes256Cipher
 * @brief Provides symmetric 256-bit AES encryption and decryption in CBC mode.
 *
 * Architectural Role:
 * - Operates within the Security layer of the UDP subsystem.
 * - Decrypts incoming datagram payloads before dispatching to message handlers.
 * - Encrypts outgoing transmission buffers before passing them to UdpSender.
 * - Uses OpenSSL EVP API with PKCS#7 padding and 128-bit initialization vectors.
 */

#ifndef AES256CIPHER_H
#define AES256CIPHER_H

#include <QByteArray>
#include <QElapsedTimer>
#include <openssl/evp.h>
#include "protocol/IrsTypes.h"

namespace GISApp::Communication::Udp::Security {

/**
 * @class Aes256Cipher
 * @brief Cryptographic cipher utility implementing AES-256-CBC.
 */
class Aes256Cipher
{
public:
    /**
     * @brief Constructs Aes256Cipher with default operational keys and IVs.
     */
    Aes256Cipher();

    /**
     * @brief Constructs Aes256Cipher with custom cryptographic key and initialization vector.
     * @param[in] key 32-byte (256-bit) binary symmetric key.
     * @param[in] iv 16-byte (128-bit) binary initialization vector.
     */
    Aes256Cipher(const QByteArray &key, const QByteArray &iv);

    /**
     * @brief Destructor releasing any retained cryptographic buffers.
     */
    ~Aes256Cipher() = default;

    /**
     * @brief Encrypts plaintext bytes using AES-256-CBC.
     * @param[in] plainText Unencrypted input buffer.
     * @return Encrypted ciphertext buffer, or empty QByteArray on failure.
     * @note Output buffer length is aligned to AES_BLOCK_SIZE (16 bytes).
     */
    QByteArray encrypt(const QByteArray &plainText);

    /**
     * @brief Decrypts ciphertext bytes using AES-256-CBC.
     * @param[in] cipherText Encrypted input buffer.
     * @return Decrypted plaintext buffer, or empty QByteArray on failure.
     */
    QByteArray decrypt(const QByteArray &cipherText);

    /**
     * @brief Updates the active 256-bit symmetric key.
     * @param[in] key 32-byte binary key.
     */
    void setKey(const QByteArray &key) { m_key = key; }

    /**
     * @brief Updates the active 128-bit initialization vector.
     * @param[in] iv 16-byte binary IV.
     */
    void setIv(const QByteArray &iv) { m_iv = iv; }

private:
    QByteArray m_key; ///< 32-byte AES-256 symmetric encryption key
    QByteArray m_iv;  ///< 16-byte initialization vector (CBC mode)
};

} // namespace GISApp::Communication::Udp::Security

#endif // AES256CIPHER_H
