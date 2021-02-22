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
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.widget.ImageView
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.core.graphics.drawable.RoundedBitmapDrawable
import androidx.core.graphics.drawable.RoundedBitmapDrawableFactory
import androidx.drawerlayout.widget.DrawerLayout
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import androidx.preference.PreferenceManager
import com.radioburst.jupitcrypt.ui.dialogs.AddDialog
import com.google.android.gms.auth.api.signin.GoogleSignIn
import com.google.android.gms.auth.api.signin.GoogleSignInClient
import com.google.android.gms.auth.api.signin.GoogleSignInOptions
import com.google.android.gms.common.api.Scope
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.navigation.NavigationView
import com.google.api.services.drive.DriveScopes
import com.squareup.picasso.Picasso
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import java.io.File
import kotlin.system.exitProcess

class MainActivity : AppCompatActivity() {

    private lateinit var appBarConfiguration: AppBarConfiguration
    private var m_Password = ""
    private var driveManager: DriveManager? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // pwd already decrypted by fingerprint
        if(intent.hasExtra("Pwd")) {
            val pwdString = intent.getStringExtra("Pwd")
            if (pwdString.isNotEmpty())
                setMasterPassword(pwdString)
        }

        setContentView(R.layout.activity_main)
        val toolbar: Toolbar = findViewById(R.id.toolbar)
        setSupportActionBar(toolbar)

        val fab: FloatingActionButton = findViewById(R.id.fab)
        fab.setOnClickListener {

            val dialog = AddDialog()
            dialog.show(supportFragmentManager, null)
        }
        val drawerLayout: DrawerLayout = findViewById(R.id.drawer_layout)
        val navView: NavigationView = findViewById(R.id.nav_view)
        val navController = findNavController(R.id.nav_host_fragment)

        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        appBarConfiguration = AppBarConfiguration(
            setOf(
                R.id.nav_home, R.id.nav_tools, R.id.nav_about
            ), drawerLayout
        )
        setupActionBarWithNavController(navController, appBarConfiguration)
        navView.setupWithNavController(navController)

        navController.addOnDestinationChangedListener { _, destination, _ ->
            if(destination.id == R.id.nav_tools || destination.id == R.id.nav_about) {
                fab.hide()
            } else {
                fab.show()
            }
        }

        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this)
        val useDrive = sharedPreferences.getBoolean("use_google", false);
        val headerView = navView.getHeaderView(0)
        val textViewName: TextView = headerView.findViewById(R.id.textView_Name)
        val textViewEmail: TextView = headerView.findViewById(R.id.textView_Email)

        if(useDrive) {
            val account = GoogleSignIn.getLastSignedInAccount(this)
            driveManager = DriveManager.getInstance(this, account!!)

            textViewName?.text = account?.displayName;
            textViewEmail?.text = account?.email;

            GlobalScope.launch(Dispatchers.IO) {
                var userIcon = Picasso.get().load(account?.photoUrl).get()

                if(userIcon != null) {
                    val roundedBitmapDrawable: RoundedBitmapDrawable =
                        RoundedBitmapDrawableFactory.create(resources, userIcon)

                    roundedBitmapDrawable.isCircular = true
                    //roundedBitmapDrawable.cornerRadius = 100.0f
                    roundedBitmapDrawable.setAntiAlias(true)
                    val image: ImageView = headerView.findViewById(R.id.imageView)
                    runOnUiThread { image?.setImageDrawable(roundedBitmapDrawable) }
                }
            }
        }
        else {
            textViewName?.text = "Jupitcrypt";
            textViewEmail?.text = "";
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        exitProcess(0)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean
    {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_settings -> {
                val navController = findNavController(R.id.nav_host_fragment)
                if (navController.currentDestination?.id != R.id.nav_tools){
                    navController.navigate(R.id.nav_tools)
                    val fab: FloatingActionButton = findViewById(R.id.fab)
                    fab.hide()
                }
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onSupportNavigateUp(): Boolean
    {
        val navController = findNavController(R.id.nav_host_fragment)
        return navController.navigateUp(appBarConfiguration) || super.onSupportNavigateUp()
    }

    fun setMasterPassword(password: String){ m_Password = password}
    fun getMasterPassword(): String = m_Password

    fun uploadPwds()
    {
        uploadFile(StartActivity.PWDSFILE)
    }

    fun uploadMasterPwd()
    {
        uploadFile(StartActivity.PWDHASHFILE)
    }

    private fun uploadFile(fileToUpload: String)
    {
        GlobalScope.launch(Dispatchers.IO) {
            val files = driveManager?.query(fileToUpload)
            files?.forEach { file ->
                driveManager?.delete(file)
                Log.d("@@@", "delete file: ${file.name}")
            }

            GlobalScope.launch(Dispatchers.IO) {
                driveManager?.upload(
                        File(
                                getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).toString(), fileToUpload
                        ),
                        "text/plain", fileToUpload
                )
                Log.d("@@@", "upload file")
                showToast()
            }
        }
    }

    fun deleteAllFiles()
    {
        GlobalScope.launch(Dispatchers.IO) {
            var files = driveManager?.query(StartActivity.PWDSFILE)
            files?.forEach { file ->
                driveManager?.delete(file)
                Log.d("@@@", "delete file: ${file.name}")
            }

            files = driveManager?.query(StartActivity.PWDHASHFILE)
            files?.forEach { file ->
                driveManager?.delete(file)
                Log.d("@@@", "delete file: ${file.name}")
            }
        }
    }

    private fun showToast()
    {
        runOnUiThread { toast("Upload done!") }
    }

    fun Context.toast(message: CharSequence) =
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show()

    fun signOut() {
        val googleSignInClient = buildGoogleSignInClient()
        googleSignInClient.signOut()
        finishAffinity()
    }

    private fun buildGoogleSignInClient(): GoogleSignInClient {
        val signInOptions = GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_SIGN_IN)
            .requestScopes(Scope(DriveScopes.DRIVE_FILE), Scope(DriveScopes.DRIVE_APPDATA))
            .build()
        return GoogleSignIn.getClient(this, signInOptions)
    }
}
