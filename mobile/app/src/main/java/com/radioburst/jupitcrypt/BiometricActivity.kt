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

package com.radioburst.jupitcrypt

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.biometric.BiometricManager
import androidx.biometric.BiometricPrompt
import androidx.core.content.ContextCompat
import java.io.File

class BiometricActivity() : AppCompatActivity() {

    private lateinit var biometricPrompt: BiometricPrompt
    private lateinit var promptInfo: BiometricPrompt.PromptInfo
    private var readyToEncrypt: Boolean = false
    private lateinit var cryptographyManager: CryptographyManager
    private lateinit var secretKeyName: String
    private lateinit var masterPwd: String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.fragment_start)

        secretKeyName = "Jupitcrypt_Key"
        masterPwd = ""

        cryptographyManager = CryptographyManager()
        biometricPrompt = createBiometricPrompt()
        promptInfo = createPromptInfo()

        if(intent.hasExtra("Decrypt"))
            authenticateToDecrypt()
        else if(intent.hasExtra("Encrypt") && intent.hasExtra("Pwd")) {

            val biometricManager = BiometricManager.from(this)
            when (biometricManager.canAuthenticate()) {
                BiometricManager.BIOMETRIC_ERROR_NO_HARDWARE, BiometricManager.BIOMETRIC_ERROR_HW_UNAVAILABLE, BiometricManager.BIOMETRIC_ERROR_NONE_ENROLLED -> {
                    setResult(Activity.RESULT_CANCELED)
                    finish()
                }
            }

            masterPwd = intent.getStringExtra("Pwd")
            authenticateToEncrypt()
        }
        else {
            setResult(Activity.RESULT_CANCELED)
            finish()
        }
    }

    private fun createBiometricPrompt(): BiometricPrompt {
        val executor = ContextCompat.getMainExecutor(this)

        val callback = object : BiometricPrompt.AuthenticationCallback() {

            override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
                super.onAuthenticationError(errorCode, errString)
                biometricPrompt.cancelAuthentication()
                setResult(Activity.RESULT_CANCELED)
                finishStartUp()
            }

            override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult) {
                super.onAuthenticationSucceeded(result)
                //Log.d(TAG, "Authentication was successful")
                processData(result.cryptoObject)
                setResult(Activity.RESULT_OK)
                finishStartUp()
            }
        }
        //The API requires the client/Activity context for displaying the prompt
        return BiometricPrompt(this, executor, callback)
    }

    private fun createPromptInfo(): BiometricPrompt.PromptInfo {
        return BiometricPrompt.PromptInfo.Builder()
            .setTitle("Unlock Jupitcrypt")
            .setDescription("")
            .setConfirmationRequired(false)
            .setNegativeButtonText("Use password")
            .build()
    }

    private fun authenticateToEncrypt() {
        readyToEncrypt = true
        if (BiometricManager.from(applicationContext).canAuthenticate() == BiometricManager
                .BIOMETRIC_SUCCESS) {
            val cipher = cryptographyManager.getInitializedCipherForEncryption(secretKeyName)
            biometricPrompt.authenticate(promptInfo, BiometricPrompt.CryptoObject(cipher))
        }
    }

    private fun authenticateToDecrypt() {
        readyToEncrypt = false
        if (BiometricManager.from(applicationContext).canAuthenticate() == BiometricManager
                .BIOMETRIC_SUCCESS) {

            val fileInitializationVector = File(this?.filesDir.toString() + "/masterpwdinitialization")

            if(fileInitializationVector.exists()) {

                val initializationVector = fileInitializationVector?.inputStream()?.readBytes()
                val cipher = cryptographyManager.getInitializedCipherForDecryption(
                    secretKeyName,
                    initializationVector
                )
                biometricPrompt.authenticate(promptInfo, BiometricPrompt.CryptoObject(cipher))
            }
            else {
                biometricPrompt.cancelAuthentication()
                setResult(Activity.RESULT_CANCELED)
                finishStartUp()
            }
        }
    }

    private fun finishStartUp() {
        if(!readyToEncrypt) {
            val i = Intent(this, MainActivity::class.java)
            if (masterPwd.isNotEmpty())
                i.putExtra("Pwd", masterPwd)
            i.action = Intent.ACTION_MAIN
            i.addCategory(Intent.CATEGORY_LAUNCHER)
            startActivity(i)
        }
        finish()
    }

    private fun processData(cryptoObject: BiometricPrompt.CryptoObject?) {
        val fileChiperText = File(this?.filesDir.toString() + "/masterpwdchipertext")
        val fileInitializationVector = File(this?.filesDir.toString() + "/masterpwdinitialization")

        if (readyToEncrypt) {
            val encryptedData = cryptographyManager.encryptData(masterPwd, cryptoObject?.cipher!!)
            fileChiperText?.outputStream()?.write(encryptedData.ciphertext)
            fileInitializationVector?.outputStream()?.write(encryptedData.initializationVector)
        } else {
            if(fileChiperText.exists()) {
                val ciphertext = fileChiperText?.inputStream()?.readBytes()
                masterPwd = cryptographyManager.decryptData(ciphertext, cryptoObject?.cipher!!)
            }
        }
    }
}