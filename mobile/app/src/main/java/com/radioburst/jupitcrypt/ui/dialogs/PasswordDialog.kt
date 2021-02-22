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

package com.radioburst.jupitcrypt.ui.dialogs

import android.app.AlertDialog
import android.app.Dialog
import android.content.DialogInterface
import android.os.Bundle
import android.os.Environment
import android.widget.EditText
import android.widget.Toast
import androidx.fragment.app.DialogFragment
import com.radioburst.jupitcrypt.MainActivity
import com.radioburst.jupitcrypt.R
import com.radioburst.jupitcrypt.ui.home.AES_256_CBC
import com.radioburst.jupitcrypt.ui.home.HomeFragment
import java.io.File

class PasswordDialog(parentFrag: HomeFragment) : DialogFragment(){

    private val mParentFrag = parentFrag
    private val mMasterPwdSaltLenght = 32

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        isCancelable = false
        return activity?.let {
            val builder = AlertDialog.Builder(it)
            // Get the layout inflater
            val inflater = requireActivity().layoutInflater

            // Inflate and set the layout for the dialog
            // Pass null as the parent view because its going in the dialog layout
            builder.setTitle("Enter password")
            val dialogView = inflater.inflate(R.layout.password_dialog, null)
            builder.setView(dialogView)

                // Add action buttons
                .setPositiveButton("Ok",
                    DialogInterface.OnClickListener { _, _ ->
                        val main = activity as MainActivity
                        val edit = dialogView.findViewById(R.id.password_dialog) as EditText

                        if(checkPassword(edit.text.toString()))
                            main.setMasterPassword(edit.text.toString())
                        else {
                            Toast.makeText(activity,
                                "Invalid password!", Toast.LENGTH_SHORT)
                                .show()
                            mParentFrag.invalidPwd()
                        }
                })
                .setNegativeButton("Cancel",
                    DialogInterface.OnClickListener { dialog, _ ->
                        dialog?.cancel()
                        activity?.finishAffinity()
                })
            builder.setCancelable(false)
            builder.create()
        } ?: throw IllegalStateException("Activity cannot be null")
    }

    private fun checkPassword(pwd: String): Boolean
    {
        val aes = AES_256_CBC
        var pwdHash: ByteArray = ByteArray(48)
        var saltBytes = ByteArray(mMasterPwdSaltLenght)
        var filedata = readPasswordHash()

        if(pwd.isEmpty())
            return false

        if(filedata.first.isEmpty()) {

            var salt = aes.generateSalt(mMasterPwdSaltLenght)
            val slatList = salt.split(",")

            for(i in 0 until mMasterPwdSaltLenght)
                saltBytes[i] = slatList[i].toInt().toByte()

            pwdHash = aes.getKeyAndIVwithPBKDF2(saltBytes, pwd)
            savePasswordHash(pwdHash, salt)
            return true
        }

        val slatList = filedata.second.split(",")
        for(i in 0 until mMasterPwdSaltLenght)
            saltBytes[i] = slatList[i].toInt().toByte()

        pwdHash = aes.getKeyAndIVwithPBKDF2(saltBytes, pwd)
        var hashString = pwdHash.fold("", { str, it -> str + "%02x".format(it) })
        if(hashString == filedata.first)
            return true
        return false
    }

    private fun readPasswordHash(): Pair<String, String>
    {
        var fileName = activity?.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString()
        fileName += "/pwd.hash"
        val hashFile = File(fileName)

        if(!hashFile.exists())
            return Pair("", "")

        var list = String(hashFile?.inputStream()?.readBytes()).split("\n")
        if(list.size > 1)
            return Pair(list[0].replace("[ChiperText]=",""), list[1].replace("[Salt]=", ""))

        return Pair("", "")
    }

    private fun savePasswordHash(pwdHash: ByteArray, salt: String)
    {
        var fileName = activity?.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString()
        fileName += "/pwd.hash"
        val hashFile = File(fileName)
        val hashString = pwdHash.fold("", { str, it -> str + "%02x".format(it) })

        hashFile?.outputStream()?.write(("[ChiperText]=$hashString\n[Salt]=$salt").toByteArray())
        mParentFrag.uploadMasterPwdHash()
    }
}