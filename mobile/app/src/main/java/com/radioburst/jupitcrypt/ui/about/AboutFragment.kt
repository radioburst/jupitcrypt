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

package com.radioburst.jupitcrypt.ui.about

import android.content.pm.PackageManager
import android.icu.text.DateFormat
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProviders
import com.radioburst.jupitcrypt.BuildConfig
import com.radioburst.jupitcrypt.R
import com.radioburst.jupitcrypt.ui.home.HomeViewModel
import java.util.*


class AboutFragment : Fragment() {

    private lateinit var homeViewModel: HomeViewModel

    override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
    ): View? {
        homeViewModel =
                ViewModelProviders.of(this).get(HomeViewModel::class.java)

        val root = inflater.inflate(R.layout.fragment_about, container, false)
        val textViewAbout = root.findViewById<TextView>(R.id.textView_Info)

        try {
            val pInfo = activity?.packageManager?.getPackageInfo(activity?.packageName!!, 0)

            val version:String = pInfo?.versionName!!
            val sdkVersion:String = pInfo.applicationInfo.targetSdkVersion.toString()
            val kotlinVersion: String = KotlinVersion.CURRENT.toString()
            val buildDate = Date(BuildConfig.BUILD_TIME.toLong())
            val strBuildDate = DateFormat.getDateInstance().format(buildDate)

            textViewAbout.text = "Jupitcrypt for Android\nv$version\n\nTarget SDK: $sdkVersion\nGoogle Drive REST API v3\n\nBuild date: $strBuildDate\nKotlin $kotlinVersion\n\nAuthor: Andi D\n\nThis program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."

        } catch (e: PackageManager.NameNotFoundException) {
            e.printStackTrace()
        }
        return root
    }

}