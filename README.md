# jupitcrypt
Jupitcrypt is a lightweight and easy to use password manager. It uses the PBKDF2 key derivation functions and AES256 to encrypt/decrypt passwords with a master password. With the google drive synchronization feature it is possible to share your passwords between devices. The Desktop App is based on the Qt Framework and OpenSSL which makes it available form most platforms. 
The mobile app is written in Kotlin thus it is only available for Android.

# Build desktop App
To build the desktop app you must install the Qt Framework. (at least 5.10)
Simple open the .pro file with the Qt-Creator and you are good to go.

## OAuth 2.0 credentials (Google Drive)
In order to use the Google Drive synchronization you must provide OAuth credentials. You can create your own API credentials within the [Google Api Console](https://console.developers.google.com/). The access token can only be retrieved by a web browser this is why jupitcrypt uses the QtWebEngine to retrieve this token. This means you have to create new In order to use the Google Drive synchronization you must provide OAuth credentials. You can create your own API credentials within the [Google Api Console](https://console.developers.google.com/). The access token can only be retrieved by a web browser this is why jupitcrypt uses the QtWebEngine to retrieve this token. This means you have to create new credentials for a web app. <br>
<br>
In the Google Api Console:
1. Create a new project
2. Create credentials
3. Choose "OAuth client ID"
4. Choose "Web application"
5. Enter a name and your redirect URIs

Should look something like this:
![api_console_web](api_console_web.png?raw=true)

After you successfully added the new credentials you can download them using the "Download Json" button.
Rename the file to client_secret.json and place it in the /desktop/resources/ folder. Now build the project again.

# Build mobile App
Simply open the folder with [Android Studio](https://developer.android.com/studio) and you should be able to build the project right away.

## OAuth 2.0 credentials (Google Drive)
Again we must provide some credentials in order to allow our app to access the Google Drive API.
This time it is a bit different. Android Apps use a SHA-1 certificate fingerprint. <br>
You must retrieve your fingerprint using the following command:

        keytool -keystore path-to-debug-or-production-keystore -list -v

Note: Debug and Release build have different fingerprints. You must create two Client ID's
<br>
In the Google Api Console:
1. Create credentials
2. Choose "OAuth client ID"
3. Choose "Android"
4. Enter a name, the package (com.radioburst.jupitcrypt) name and your fingerprint

Should look something like this:
![api_console_android](api_console_android.png?raw=true)

Now you should be able to access the Google Drive API with the jupitcrypt App.

# License
This project is under the GNU General Public License v3 <br>
This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit (http://www.openssl.org/)

