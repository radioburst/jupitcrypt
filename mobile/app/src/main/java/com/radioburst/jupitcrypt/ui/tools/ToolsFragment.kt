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

package com.radioburst.jupitcrypt.ui.tools

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.text.InputType
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.preference.EditTextPreference
import androidx.preference.Preference
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.SwitchPreferenceCompat
import com.radioburst.jupitcrypt.BiometricActivity
import com.radioburst.jupitcrypt.MainActivity
import com.radioburst.jupitcrypt.R
import com.radioburst.jupitcrypt.StartActivity


class ToolsFragment : PreferenceFragmentCompat() {

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        //super.onCreate(savedInstanceState)
        setPreferencesFromResource(R.xml.preferences, rootKey)

        val editTextPreferencepwdlength = findPreference<EditTextPreference>("pwd_length")
        editTextPreferencepwdlength?.setOnBindEditTextListener { editText ->
            editText.inputType = InputType.TYPE_CLASS_NUMBER
        }

        val editTextPreferencewrongentrylimit = findPreference<EditTextPreference>("wrong_entry_limit")
        editTextPreferencewrongentrylimit?.setOnBindEditTextListener { editText ->
            editText.inputType = InputType.TYPE_CLASS_NUMBER
        }

        val logoutButton:Preference? = findPreference<Preference>("logout_drive")
        logoutButton?.setOnPreferenceClickListener {
            (activity as MainActivity).signOut()
            true
        }

        val deleteButton:Preference? = findPreference<Preference>("delete_drive")
        deleteButton?.setOnPreferenceClickListener {

            val builder = activity?.let { AlertDialog.Builder(it) }
            builder?.setTitle("Delete from Google Drive")
            builder?.setMessage("Do you really want to delete all your passwords? This can't be undone!")

            builder?.setPositiveButton(android.R.string.yes) { _, _ ->
                (activity as MainActivity).deleteAllFiles()
            }
            builder?.setNegativeButton(android.R.string.no) { _, _ ->
                // to nothing
            }
            val dialog: AlertDialog = builder!!.create()
            dialog.show()

            true
        }

        val useDrive: SwitchPreferenceCompat? = findPreference("use_google")
        useDrive!!.onPreferenceChangeListener =
            Preference.OnPreferenceChangeListener { pref, newValue ->
                if(newValue.toString() == "true")
                {
                    val i = Intent(activity, StartActivity::class.java)
                    i.putExtra("Dialog", "")
                    i.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP)
                    startActivity(i)
                }
                true
            }

        val useBiometric: SwitchPreferenceCompat? = findPreference("use_biometrics")
        useBiometric!!.onPreferenceChangeListener =
            Preference.OnPreferenceChangeListener { pref, newValue ->
                if(newValue.toString() == "true") {

                    val i = Intent(activity, BiometricActivity::class.java)
                    i.putExtra("Encrypt", "")
                    i.putExtra("Pwd", (activity as MainActivity).getMasterPassword())

                    startActivityForResult(i, 1)
                }
                true
            }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)

        if (requestCode == 1 && resultCode == Activity.RESULT_CANCELED) {
            val useBiometric: SwitchPreferenceCompat? = findPreference("use_biometrics")
            useBiometric?.isChecked = false
            Toast.makeText(activity, "Error while enabling Biometrics!", Toast.LENGTH_SHORT).show()
        }
    }
}