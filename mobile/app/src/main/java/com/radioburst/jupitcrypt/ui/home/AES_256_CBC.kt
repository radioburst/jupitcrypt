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

package com.radioburst.jupitcrypt.ui.home

import java.io.UnsupportedEncodingException
import java.security.GeneralSecurityException
import java.security.SecureRandom
import javax.crypto.*
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.PBEKeySpec
import javax.crypto.spec.SecretKeySpec


object AES_256_CBC {
    private const val INDEX_KEY = 0
    private const val INDEX_IV = 32
    private const val ITERATIONS = 200000
    private const val KEY_SIZE_BITS = 256
    private const val IV_SIZE_BITS = 128

    fun decrypt(encrypted: String, salt: ByteArray, password: String): String {
        return try {
            val aesCBC = Cipher.getInstance("AES/CBC/PKCS5Padding")
            val keyAndIV = getKeyAndIVwithPBKDF2(salt, password)
            val key = SecretKeySpec(keyAndIV.copyOfRange(INDEX_KEY, INDEX_IV), "AES")
            val iv = IvParameterSpec(keyAndIV.copyOfRange(INDEX_IV, (KEY_SIZE_BITS + IV_SIZE_BITS) / 8))

            // --- initialize cipher instance and decrypt ---
            aesCBC.init(Cipher.DECRYPT_MODE, key, iv)
            val decrypted = aesCBC.doFinal(decodeHexString(encrypted))

            String(decrypted, charset("ASCII"))
        } catch (e: BadPaddingException) { // AKA "something went wrong"
            throw IllegalStateException(
                    "Bad password, algorithm, mode or padding;"
                            + " no salt, wrong number of iterations or corrupted ciphertext."
            )
        } catch (e: IllegalBlockSizeException) {
            throw IllegalStateException(
                    "Bad algorithm, mode or corrupted (resized) ciphertext."
            )
        } catch (e: GeneralSecurityException) {
            throw IllegalStateException(e)
        } catch (e: UnsupportedEncodingException) {
            throw IllegalStateException(e)
        }
    }

    fun encrypt(decrypted: String, salt: ByteArray, password: String): String {
        return try {
            val aesCBC = Cipher.getInstance("AES/CBC/PKCS5Padding")
            val keyAndIV = getKeyAndIVwithPBKDF2(salt, password)
            val key = SecretKeySpec(keyAndIV.copyOfRange(INDEX_KEY, INDEX_IV), "AES")
            val iv = IvParameterSpec(keyAndIV.copyOfRange(INDEX_IV, (KEY_SIZE_BITS + IV_SIZE_BITS) / 8))

            // --- initialize cipher instance and decrypt ---
            aesCBC.init(Cipher.ENCRYPT_MODE, key, iv)
            val encrypted = aesCBC.doFinal(decrypted.toByteArray(charset("ASCII")))

            var retStr = ""
            for (b in encrypted) {
                retStr += String.format("%02x", b)
            }
            retStr
    } catch (e: BadPaddingException) { // AKA "something went wrong"
        throw IllegalStateException(
                "Bad password, algorithm, mode or padding;"
                        + " no salt, wrong number of iterations or corrupted ciphertext."
        )
    } catch (e: IllegalBlockSizeException) {
        throw IllegalStateException(
                "Bad algorithm, mode or corrupted (resized) ciphertext."
        )
    } catch (e: GeneralSecurityException) {
        throw IllegalStateException(e)
    } catch (e: UnsupportedEncodingException) {
        throw IllegalStateException(e)
    }
    }

    fun getKeyAndIVwithPBKDF2(salt: ByteArray, password: String) : ByteArray {
        val skf: SecretKeyFactory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA512")
        val spec = PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_SIZE_BITS + IV_SIZE_BITS)
        return skf.generateSecret(spec)?.encoded!!
    }

    private fun decodeHexString(hexString: String): ByteArray? {
        require(hexString.length % 2 != 1) { "Invalid hexadecimal String supplied." }
        val bytes = ByteArray(hexString.length / 2)
        var i = 0
        while (i < hexString.length) {
            bytes[i / 2] = hexToByte(hexString.substring(i, i + 2))
            i += 2
        }
        return bytes
    }

    private fun hexToByte(hexString: String): Byte {
        val firstDigit: Int = toDigit(hexString[0])
        val secondDigit: Int = toDigit(hexString[1])
        return ((firstDigit shl 4) + secondDigit).toByte()
    }

    private fun toDigit(hexChar: Char): Int {
        val digit = Character.digit(hexChar, 16)
        require(digit != -1) { "Invalid Hexadecimal Character: $hexChar" }
        return digit
    }

    fun generateSalt(saltLenght: Int): String{
        var salt = ""
        var saltbyte  = ByteArray(1)
        val secureRandom = SecureRandom()

        for (i in 0 until saltLenght) {
            secureRandom.nextBytes(saltbyte)
            salt += (saltbyte[0] + 128).toString()

            if(i != saltLenght - 1)
                salt += ","
        }
        return salt
    }
}
