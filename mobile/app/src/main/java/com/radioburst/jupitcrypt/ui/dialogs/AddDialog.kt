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
import android.content.Context
import android.content.DialogInterface
import android.os.Bundle
import android.os.Environment
import android.text.InputType
import android.view.View
import android.view.inputmethod.InputMethodManager
import android.widget.Button
import android.widget.EditText
import android.widget.Toast
import androidx.fragment.app.DialogFragment
import androidx.navigation.findNavController
import androidx.preference.PreferenceManager
import com.radioburst.jupitcrypt.MainActivity
import com.radioburst.jupitcrypt.R
import com.radioburst.jupitcrypt.ui.home.AES_256_CBC
import com.radioburst.jupitcrypt.ui.home.JsonModel
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import java.io.File
import kotlin.math.floor

class AddDialog : DialogFragment(){

    private var saltLenght = 16;

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        return activity?.let {
            val builder = AlertDialog.Builder(it)
            // Get the layout inflater
            val inflater = requireActivity().layoutInflater

            // Inflate and set the layout for the dialog
            // Pass null as the parent view because its going in the dialog layout
            builder.setTitle("Add password")
            val dialogView = inflater.inflate(R.layout.add_dialog, null)

            val generateButton = dialogView.findViewById(R.id.button_gen) as Button
            val editPwd = dialogView.findViewById(R.id.password) as EditText

            val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(activity)
            val showPwd = sharedPreferences.getBoolean("show_pwd", false);
            if(showPwd)
                editPwd.inputType = InputType.TYPE_CLASS_TEXT

            generateButton.setOnClickListener {
                editPwd.setText(generateRandomPassword())
            }

            builder.setView(dialogView)
                // Add action buttons
                .setPositiveButton("Add",
                    DialogInterface.OnClickListener { _, _ ->

                        val desc = dialogView.findViewById(R.id.description) as EditText
                        val pwd = dialogView.findViewById(R.id.password) as EditText
                        val user = dialogView.findViewById(R.id.editTextUser) as EditText
                        okClicked(desc.text.toString(), pwd.text.toString(), user.text.toString())
                    })
                .setNegativeButton("Cancel",
                    DialogInterface.OnClickListener { dialog, _ ->
                        dialog.cancel()
                    })
            builder.create()
        } ?: throw IllegalStateException("Activity cannot be null")
    }

    private fun okClicked(textdesc: String, password: String, user: String) {

        if(addEntryToJson(textdesc, password, user)) {

            Toast.makeText(activity, "$textdesc was added", Toast.LENGTH_SHORT).show()
            val navController = activity?.findNavController(R.id.nav_host_fragment)
            navController?.navigate(R.id.nav_home)
            view?.hideKeyboard()
        }
        else
            Toast.makeText(activity, "Error while adding $textdesc!", Toast.LENGTH_SHORT).show()
    }

    private fun addEntryToJson(textdesc: String, password: String, user: String): Boolean
    {
        var entries = parseJson()
        val aes = AES_256_CBC
        val salt = aes.generateSalt(saltLenght)
        val gson = Gson()
        var saltBytes = ByteArray(saltLenght)
        var encrypted = ""
        val slatList = salt.split(",")
        if(slatList.count() > saltLenght - 1) {

            for(i in 0 until saltLenght)
                saltBytes[i] = slatList[i].toInt().toByte()

            encrypted = aes.encrypt(password, saltBytes, (activity as MainActivity).getMasterPassword())
        }

        entries.add(JsonModel(textdesc, encrypted, salt, user))

        var fileName = activity?.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString()
        fileName += "/pwds.json"

        File(fileName)?.outputStream()?.write(gson.toJson(entries).toByteArray())

        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(activity)
        val useDrive = sharedPreferences.getBoolean("use_google", false);

        if(useDrive) {
            val act = activity as MainActivity
            act.uploadPwds()
        }
        return true
    }

    private fun parseJson(): ArrayList<JsonModel>{

        var fileName = activity?.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString()
        fileName += "/pwds.json"
        val file = File(fileName)

        if(!file.exists())
            return ArrayList<JsonModel>()

        val jsonList = file?.inputStream()?.readBytes()?.toString(Charsets.UTF_8)
        val gson = Gson()
        val array = object : TypeToken<ArrayList<JsonModel>>() {}.type

        return gson.fromJson(jsonList, array)
    }

    private fun generateRandomPassword(): String {

        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(activity)
        val pwdLength = sharedPreferences.getString("pwd_length", "24");
        val useNumbers = sharedPreferences.getBoolean("use_numbers", true);
        val useSpecialChars = sharedPreferences.getBoolean("use_special_c", true);
        var specialChars: String? = ""
        if(useSpecialChars)
            specialChars = sharedPreferences.getString("special_chars", "#%&!?*+-=$()/_:;,.@")
        var chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

        if(useNumbers)
            chars += "0123456789"
        if(useSpecialChars)
            chars += specialChars

        var passWord = ""
        for (i in 0..pwdLength!!.toInt()) {
            passWord += chars[floor(Math.random() * chars.length).toInt()]
        }
        return passWord
    }

    private fun View.hideKeyboard() {
        val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.hideSoftInputFromWindow(windowToken, 0)
    }
}