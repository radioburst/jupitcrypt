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

import android.annotation.SuppressLint
import android.app.AlertDialog
import android.app.Dialog
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.DialogInterface
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.fragment.app.DialogFragment
import androidx.fragment.app.FragmentManager
import com.radioburst.jupitcrypt.R


class InformationDialog: DialogFragment(){

    private var mPassword = ""
    private var mDescription = ""
    private var mUser: String = ""

    @SuppressLint("SetTextI18n")
    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        return activity?.let {
            val builder = AlertDialog.Builder(it)
            // Get the layout inflater
            val inflater = requireActivity().layoutInflater

            // Inflate and set the layout for the dialog
            // Pass null as the parent view because its going in the dialog layout
            builder.setTitle(mDescription)
            val dialogView = inflater.inflate(R.layout.information_dialog, null)
            dialogView.findViewById<TextView>(R.id.textView_pwd).text = mPassword

            if(mUser.isNotEmpty())
                dialogView.findViewById<TextView>(R.id.textView_user).text = mUser
            else
            {
                dialogView.findViewById<TextView>(R.id.textView_user).visibility = View.GONE
                dialogView.findViewById<TextView>(R.id.textView_user_heading).visibility = View.GONE
            }

            val button = dialogView.findViewById<Button>(R.id.button_clipboard)
            button.setOnClickListener {

                context?.copyToClipboard(mPassword)
                dialog?.cancel()
                Toast.makeText(activity, "Copied password to clipboard", Toast.LENGTH_SHORT).show()
            }

            builder.setView(dialogView)
                // Add action buttons
                .setPositiveButton("Close",
                    DialogInterface.OnClickListener { dialog, _ ->
                        dialog.cancel()
                    })
            builder.create()
        } ?: throw IllegalStateException("Activity cannot be null")
    }

    fun show(manager: FragmentManager?, tag: String?, password: String, description: String, user: String?) {
        super.show(manager!!, tag)
        mPassword = password
        mDescription = description
        if(!user.isNullOrEmpty())
            mUser = user
    }

    fun Context.copyToClipboard(text: CharSequence){
        var clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
        val clip = ClipData.newPlainText("text", text)
        clipboard.setPrimaryClip(clip)
    }
}