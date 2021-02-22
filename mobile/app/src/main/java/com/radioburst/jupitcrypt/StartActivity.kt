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

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.os.Environment
import android.util.Log
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.PreferenceManager
import com.google.android.gms.auth.api.signin.GoogleSignIn
import com.google.android.gms.auth.api.signin.GoogleSignInClient
import com.google.android.gms.auth.api.signin.GoogleSignInOptions
import com.google.android.gms.common.api.Scope
import com.google.api.services.drive.DriveScopes
import kotlinx.coroutines.*
import java.io.File
import java.text.SimpleDateFormat

class StartActivity() : AppCompatActivity() {

    private var driveManager: DriveManager? = null

    @SuppressLint("SimpleDateFormat")
    private val dateFormat = SimpleDateFormat("yyyy-MM-dd hh:mm:ss")
    private var mRestore = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.fragment_start)

        if(intent.hasExtra("Dialog")) {

            val builder = AlertDialog.Builder(this)
            builder?.setTitle("Restore from Google Drive")
            builder?.setMessage("Restore from google drive? Only hit \"Restore\" if you have already synced to Drive! Choose \"Upload\" if this is your first time using the Drive sync.")

            builder?.setPositiveButton("Restore") { _, _ ->
                mRestore = true
                startAuth()
            }
            builder?.setNegativeButton("Upload") { _, _ ->
                mRestore = false
                startAuth()
            }

            val dialog: AlertDialog = builder!!.create()
            dialog.show()
        }
        else
            startAuth()
    }

    private fun startAuth()
    {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this)
        val useDrive = sharedPreferences.getBoolean("use_google", false);

        if(useDrive)
            signIn()
        else
            finishSync()
    }

    private fun signIn() {
        val googleSignInClient = buildGoogleSignInClient()
        startActivityForResult(googleSignInClient.signInIntent, REQUEST_CODE_SIGN_IN)
    }

    private fun buildGoogleSignInClient(): GoogleSignInClient {
        val signInOptions = GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_SIGN_IN)
            .requestScopes(Scope(DriveScopes.DRIVE_FILE), Scope(DriveScopes.DRIVE_APPDATA), Scope("email"))
            .build()
        return GoogleSignIn.getClient(this, signInOptions)
    }

    private fun upload(InputFiles: List<String>) {
        GlobalScope.launch(Dispatchers.IO) {
            InputFiles?.forEach { inputFile ->

                val files = driveManager?.query(inputFile)
                files?.forEach { file ->
                    driveManager?.delete(file)
                    Log.d("@@@", "delete file: ${file.name}")
                }

                driveManager?.upload(
                        File(getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString(), inputFile),
                        "text/plain",
                        inputFile
                )
                Log.d("@@@", "upload file")
            }
            withContext (Dispatchers.Main) {
                finishSync() // runs in main thread
            }
        }
    }

    private fun restore(InputFiles: List<String>) {
        GlobalScope.launch(Dispatchers.IO) {
            InputFiles?.forEach { inputFile ->
                val files = driveManager?.query(inputFile)
                files?.forEach { file ->
                    val localFile = File(getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString(), inputFile)
                    Log.d(
                            "@@@",
                            "before restore local file last modified time: ${dateFormat.format(localFile.lastModified())}"
                    )
                    deleteLocal(inputFile)
                    driveManager?.download(file.id, localFile.absolutePath)
                    Log.d(
                            "@@@", "restore file: ${file.name}, " +
                            "local file modified time : ${dateFormat.format(localFile.lastModified())}"
                    )
                }
            }
            withContext (Dispatchers.Main) {
                finishSync() // runs in main thread
            }
        }
    }

    private fun find(inputFile: String) {
        GlobalScope.launch(Dispatchers.IO) {
            val files = driveManager?.query(inputFile)
            Log.d("@@@", "find files size: ${files?.size}")
            files?.forEach {
                Log.d("@@@", "find file: ${it.name}")
            }
        }
    }

    private fun delete(inputFile: String) {
        GlobalScope.launch(Dispatchers.IO) {
            val files = driveManager?.query(inputFile)
            files?.forEach { file ->
                driveManager?.delete(file)
                Log.d("@@@", "delete file: ${file.name}")
            }
        }
    }

    private fun createLocal(inputFile: String) {
        val file = File(getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString(), inputFile)
        if (!file.exists()) {
            file.createNewFile()
            file.writeText("file created by use click")
            Log.d("@@@", "create file")
        } else {
            Log.d("@@@", "local file exists")
        }
    }

    private fun findLocal(inputFile: String) {
        val file = File(getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString(), inputFile)
        Log.d("@@@", "find local file: ${file.exists()}")
    }

    private fun deleteLocal(inputFile: String) {
        File(getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString(), inputFile).delete()
        Log.d("@@@", "delete local file")
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == REQUEST_CODE_SIGN_IN && resultCode == Activity.RESULT_OK) {
            val account = GoogleSignIn.getLastSignedInAccount(this)
            driveManager = DriveManager.getInstance(this, account!!)

            if(mRestore)
                restore(listOf(PWDSFILE, PWDHASHFILE))
            else
                upload(listOf(PWDSFILE, PWDHASHFILE))
        }
    }

    companion object {
        const val REQUEST_CODE_SIGN_IN = 0

        const val PWDSFILE = "pwds.json"
        const val PWDHASHFILE = "pwd.hash"
    }

    private fun finishSync(){

        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this)
        val useBiometrics = sharedPreferences.getBoolean("use_biometrics", false);

        if(useBiometrics) {
            val i = Intent(this, BiometricActivity::class.java)
            i.putExtra("Decrypt", "")
            startActivity(i)
        }
        else {
            val i = Intent(this, MainActivity::class.java)
            i.action = Intent.ACTION_MAIN
            i.addCategory(Intent.CATEGORY_LAUNCHER)
            startActivity(i)
        }
        finish()
    }
}