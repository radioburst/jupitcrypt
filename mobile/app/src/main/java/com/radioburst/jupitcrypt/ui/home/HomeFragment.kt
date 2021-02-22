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

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.os.Bundle
import android.os.Environment
import android.view.*
import android.view.ContextMenu.ContextMenuInfo
import android.widget.AdapterView.AdapterContextMenuInfo
import android.widget.ArrayAdapter
import android.widget.ListView
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProviders
import androidx.preference.PreferenceManager
import com.radioburst.jupitcrypt.MainActivity
import com.radioburst.jupitcrypt.R
import com.radioburst.jupitcrypt.ui.dialogs.PasswordDialog
import com.radioburst.jupitcrypt.ui.dialogs.InformationDialog
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import java.io.File


class HomeFragment : Fragment() {

    private lateinit var homeViewModel: HomeViewModel
    private var mInvalidCount = 0
    private var saltLenght = 16;

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        homeViewModel =
            ViewModelProviders.of(this).get(HomeViewModel::class.java)
        val root = inflater.inflate(R.layout.fragment_home, container, false)

        val view = root.findViewById(R.id.pwdList) as ListView

        if((activity as MainActivity).getMasterPassword().isEmpty()) {
            val dialog = PasswordDialog(this)
            dialog.show(parentFragmentManager, null)
        }

        registerForContextMenu(view)

        view.setOnItemClickListener { _, _, position, _ ->

            onItemClicked(view.getItemAtPosition(position) as JsonModel)
        }

        fillTree(view)
        return root
    }

    override fun onCreateContextMenu(menu: ContextMenu, v: View, menuInfo: ContextMenuInfo?) {
        if (v.id == R.id.pwdList) {

            menu.add(1 , 1, 1, "Delete")
            menu.add(1, 2, 2,"Show")
            menu.add(1, 3, 3,"Copy to clipboard")
        }
    }

    override fun onContextItemSelected(item: MenuItem): Boolean {

        val info = item!!.menuInfo as AdapterContextMenuInfo
        val viewItem = view?.findViewById<ListView>(R.id.pwdList)?.getItemAtPosition(info.position) as JsonModel

        when (item!!.itemId) {
            1 -> {

                deleteItem(viewItem)
                return true
            }
            2 -> {
                onItemClicked(viewItem)
                return true
            }
            3 -> {
                copyPasswordToClipboard(viewItem)
                return true
            }
            else -> return false
        }
    }

    private fun fillTree(view: ListView){

        var entries: ArrayList<JsonModel> = parseJson()

        if(entries.isEmpty())
            return
        val pwdAdapter = ArrayAdapter(activity as MainActivity, android.R.layout.simple_list_item_1, entries)
        view.adapter = pwdAdapter
    }

    private fun parseJson(): ArrayList<JsonModel>{

        var fileName = activity?.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString()
        fileName += "/pwds.json"
        val file = File(fileName)

        if(!file.exists())
            return ArrayList<JsonModel>()

        //var jsonList = FileContent("text/plain", file)?.toString()
        val jsonList = file.readText()
       // val jsonList = File(fileName)?.inputStream()?.readBytes()?.toString(Charsets.UTF_8)

        if(jsonList.isEmpty())
            return ArrayList<JsonModel>()

        val gson = Gson()
        val array = object : TypeToken<ArrayList<JsonModel>>() {}.type

        return gson.fromJson(jsonList, array)
    }

    private fun onItemClicked(item :JsonModel) {

        val aes = AES_256_CBC
        val salt = item?.Salt.split(",")
        var saltBytes = ByteArray(saltLenght)
        if(salt.count() > saltLenght - 1) {

            for(i in 0 until saltLenght)
                saltBytes[i] = salt[i].toInt().toByte()

            val decrypt = aes.decrypt(item?.Ciphertext, saltBytes, (activity as MainActivity).getMasterPassword())

            val dialog = InformationDialog()
            dialog.show(requireFragmentManager(), null, decrypt, item.Description, item.User)
        }

    }

    private fun longToUInt32ByteArray(value: Long): ByteArray {
        val bytes = ByteArray(4)
        bytes[3] = (value and 0xFFFF).toByte()
        bytes[2] = ((value ushr 8) and 0xFFFF).toByte()
        bytes[1] = ((value ushr 16) and 0xFFFF).toByte()
        bytes[0] = ((value ushr 24) and 0xFFFF).toByte()
        return bytes
    }

    private fun deleteItem(item: JsonModel)
    {
        removeEntryFromJson(item)

        val listview = view?.findViewById<ListView>(R.id.pwdList)

        if(listview != null)
            fillTree(listview)

        Toast.makeText(getActivity(), "${item.Description} was deleted!", Toast.LENGTH_SHORT).show()
    }

    private fun removeEntryFromJson(item: JsonModel): Boolean
    {
        var entries = parseJson()
        val gson = Gson()

        for(pos in entries.indices) {

            if(entries[pos].Description == item.Description) {
                entries.removeAt(pos)
                break
            }
        }
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

    private fun copyPasswordToClipboard(item: JsonModel)
    {
        val aes = AES_256_CBC
        val salt = item?.Salt.split(",")
        var saltBytes = ByteArray(saltLenght)
        if(salt.count() > saltLenght - 1) {

            for(i in 0 until saltLenght)
                saltBytes[i] = salt[i].toInt().toByte()

            val decrypt = aes.decrypt(item?.Ciphertext, saltBytes, (activity as MainActivity).getMasterPassword())
            context?.copyToClipboard(decrypt)
            Toast.makeText(getActivity(), "Copied password to clipboard", Toast.LENGTH_SHORT).show()
        }
    }

    fun Context.copyToClipboard(text: CharSequence){
        var clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
        val clip = ClipData.newPlainText("text", text)
        clipboard.setPrimaryClip(clip)
    }

    fun invalidPwd()
    {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(activity)
        val maxInvalid = sharedPreferences.getString("wrong_entry_limit", "3");
        mInvalidCount++;

        if(mInvalidCount > maxInvalid!!.toInt() - 1)
            activity?.finishAffinity()
        else {
            val dialog = PasswordDialog(this)
            dialog.show(parentFragmentManager, null)
        }
    }

    fun uploadMasterPwdHash()
    {
        (activity as MainActivity).uploadMasterPwd()
    }
}
