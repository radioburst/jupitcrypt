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

import android.content.Context
import com.google.android.gms.auth.api.signin.GoogleSignInAccount
import com.google.api.client.extensions.android.http.AndroidHttp
import com.google.api.client.googleapis.extensions.android.gms.auth.GoogleAccountCredential
import com.google.api.client.http.FileContent
import com.google.api.client.json.jackson2.JacksonFactory

import com.google.api.services.drive.Drive
import com.google.api.services.drive.DriveScopes
import com.google.api.services.drive.model.File
import java.io.FileOutputStream
import java.util.*

class DriveManager private constructor(context: Context, googleAccount :GoogleSignInAccount) {

    companion object : SingletonHolder<DriveManager, Context, GoogleSignInAccount>(::DriveManager) {
        const val FOLDER_MIME_TYPE = "application/vnd.google-apps.drive-sdk"
    }

    private val drive: Drive = Drive.Builder(
        AndroidHttp.newCompatibleTransport(),
        JacksonFactory.getDefaultInstance(),
        GoogleAccountCredential.usingOAuth2(
            context,
            listOf(DriveScopes.DRIVE_FILE, DriveScopes.DRIVE_APPDATA))
            .setSelectedAccount(googleAccount.account)
    ).setApplicationName(context.getString(R.string.app_name))
        .build()

    fun upload(
        file: java.io.File,
        mimeType: String,
        name: String,
        block: (Drive.Files.Create.() -> Drive.Files.Create)? = null
    ): File {
        val fileMeta = File()
        fileMeta.name = name
        fileMeta.parents = Collections.singletonList("appDataFolder")
        val fileContent = FileContent(mimeType, file)
        var create = drive.Files()
            .create(fileMeta, fileContent)
        create = if (block != null) {
            create.block()
        } else {
            create
        }
        return create.execute()
    }

    fun download(fileId: String, filePath: String) {
        val outputStream = FileOutputStream(filePath)
        drive.files()
            .get(fileId)
            .executeMediaAndDownloadTo(outputStream)
    }

    fun isAppFolderExists(): Boolean {
        val appDataFolder = drive.files().list()
            .setSpaces("appDataFolder")
            .execute()
        return appDataFolder != null && appDataFolder.isNotEmpty()
    }

    fun query(name: String): List<File>? {
        val queryString = "name = '$name'"
        val queryResult = drive.files()
            .list()
            .setQ(queryString)
            .setSpaces("appDataFolder")
            .execute()
        return queryResult.files
    }

    fun delete(file: File) {
        drive.files().delete(file.id).execute()
    }

    fun queryChildren(
        folder: File,
        fileName: String
    ): List<File>? {
        val folderQuery = Query.build {
            filters = mutableListOf<String>().apply {
                add("mimeType = 'application/vnd.google-apps.drive-sdk'")
                add("name = '${folder.name}'")
            }
        }
        val folders = drive.files()
            .list()
            .setQ(folderQuery.toString())
            .execute()
        val result = mutableListOf<File>()
        folders.files.forEach {
            val fileQuery = Query.build {
                filters = mutableListOf<String>().apply {
                    add("mimeType != 'application/vnd.google-apps.folder'")
                    add("'${it.id}' in parents")
                    add("name = '$fileName'")
                }
            }
            val queryResult = drive.files()
                .list()
                .setQ(fileQuery.toString())
                .execute()
            result.addAll(queryResult.files)
        }
        return result
    }
}