##Instructions for translators  

* Download and install [QT Linguist](https://qtlinguistdownload.googlecode.com/files/QtLinguist-4.6.0-Win32-Installer.exe)
* Rename the file picsdl\_blank\_translation.ts into picsdl\_xx.ts where xx is the [language code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
* Launch QT Linguist
* Click on File/Open (or hit Ctrl+O) and browse to your file picsdl_xx.ts
* Perform the translation with QT Linguist. See the [QT Linguist manual](http://doc.qt.io/qt-5.5/linguist-translators.html) for help.
* Once you are finished:
    * save the TS file
    * send it to clement@nardi.fr, or submit a Pull-Request to this repository

###Instructions for developers

* Make sure the new TS file is well placed in the folder Translations
* edit PicsDL.pro, and add this new TS file to the definition of TRANSLATIONS
* Download a flag icon from [here](https://www.iconfinder.com/iconsets/Flag), choose size 32x32
* rename the downloaded file into flag_xx.png and move it to the folder PicsDL/resources/
* add this new png file to resources.qrc (from QT Creator: right-click/open in editor)
* Rebuild
