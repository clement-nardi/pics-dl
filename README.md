**Tired of spending 10 minutes each time you want to retrieve pictures from your camera?**  
**With PicsDL, you just plug your device, and... done!**  

##Main features:  
* Retrieves pictures and videos from any type of device (camera, smartphone, memory card, USB stick, External Hard Drive).
* All-automatic mode: simply plug the device and watch your pictures being downloaded and organized.
* Organize and rename your files based on date/time and EXIF tags (such as camera brand/name, etc.)
* Geo-tag the pictures
  
---
  
The pics-dl project is composed of of two modules and has 3 dependencies:

##Modules:  
* PicsDL: the main application  
* WPDInterface: a library to access smartphones and cameras, based on Microsoft's Windows Portable Device API
  
##External dependencies:  
* libexif: used to read exif tags and thumbnails from picture files
* exiftool: used to geo-tag the pictures
* o2: OAuth library for QT
  
##Technical constraints:  
* The WPDInterface library can only be built with MS Visual Studio
* PicsDL and the libexif must be built with the same compiler. If you intend to build with static linking, it must even be the exact same compiler version.
* libexif only provides compilation procedure using MinGW/MSYS, so it is strongly advised to compile PicsDL with MinGW (using QT Creator).
 

##Note:  
You don't need to install visual studio and compile WPDInterface.dll in order to work on PicsDL.  
This is because the dll is versionned along with its code: */pics-dl/WPDInterface/Release/WPDInterface.dll  


#Step-by-step guide to compile PicsDL:

##1. Downloads
* [QT 5.4](http://download.qt.io/official_releases/qt/5.4/5.4.0/qt-opensource-windows-x86-mingw491_opengl-5.4.0.exe)
* [MSVC redistributable 2013](http://www.microsoft.com/en-us/download/confirmation.aspx?id=40784)
* [Inno Setup](http://www.jrsoftware.org/download.php/is.exe)
* [Strawberry perl](http://strawberryperl.com/download/5.20.1.1/strawberry-perl-5.20.1.1-32bit.msi)
* [Git](http://git-scm.com/download/win)

Optional but recommended:  
* [notepad++](http://download.tuxfamily.org/notepadplus/6.6.9/npp.6.6.9.Installer.exe)

##2. Install

###2.1 QT  
On the 3rd step, make sure you select all the Tools (including MinGW).  
Otherwise just click on Next  

###2.2 Notepad++    
Just click Next.  

###2.3 Microsoft Visual C++ Redistributable Packages for Visual Studio 2013  
Just click Next.  

###2.4 Perl and Exiftool module
**Must be installed before Git**  
Install Strawberry Perl, leave all default settings.  
Once installed, launch "CPAN Client", and write in the terminal:  

###2.5 Git  
**Must be installed after Strawberry Perl**  
**At the 4th step, select the 3rd option**: "Use Git and optional Unix tools from the Windows Command Prompt"
Otherwise Just click Next.  

    install Image::ExifTool

###2.7 Inno Setup 5  
Just Click Next (leave default options)  
Note: Inno Setup is used to generate the windows installer.   
If you don't want to generate the installer, you will need to remove the last line in the .pro file  

##3. Download the sources  
Choose a folder where to download the sources  
right-click on this folder and choose "Git Bash"  
In the terminal, write:  

    git clone https://github.com/clement-nardi/pics-dl.git  

##5. Compile PicsDL  
Double-click on */pics-dl/PicsDL/PicsDL.pro  
let QT Creator configure the project automatically  
click on the compile button (green triangle)  

##6. (optional, 2 hours, recommended) Compile PicsDL with static linking  
This is strongly recommended if you plan to produce an installer for PicsDL (the final file will be much smaller).    
You just need to execute de script "*/Qt/launch-static-buid.ps1" (Right-click, execute with PowerShell).  
inspired from: http://qt-project.org/wiki/How-to-build-a-static-Qt-for-Windows-MinGW  
Note: the current Inno setup script is only designed to work with a statically linked executable  


#Step-by-step guide to compile the WPDInterface dll   

##1. Install Microsoft Visual Studio Express 2013 for desktop  
Follow the instructions here: http://go.microsoft.com/?linkid=9832280&clcid=0x409  
Note 1: you need a microsoft account  
Note 2: you need at least Windows 7 SP1, with IE11  

##2. Install the Windows Driver Kit Version 7.1.0  
Note: this is needed due to dependencies on the ATL, which is not included in later versions of the WDK.  
Follow the instructions here: http://www.microsoft.com/en-us/download/details.aspx?id=11800  
You may need some disk image mounting software, such as WinCDEmu: http://wincdemu.sysprogs.org/download/  

##3. (optional) Install the Windows Driver Kit 8.1  
Note: this contains the latest version of the Portable Device API  
Follow the instructions here:http://www.microsoft.com/en-us/download/details.aspx?id=42273  

##4. Build the dll   
double-click on */pics-dl/WPDInterface/WPDInterface.sln  
click on the Build button (green triangle)  


#(Optional) Step-by-step guide to compile libexif

##1. Downloads
* [MSYS/MinGW](https://sourceforge.net/projects/mingw/files/latest/download)
 
##2 Install MinGW/MSYS  
The full install procedure is [here](http://www.mingw.org/wiki/Getting_Started)  
Short install procedure:  
- Leave default options and click Next until you reach the "Installation Manager"  
- right-click on mingw-developer-toolkit, and mark for installation  
- go to menu Installation / Apply Changes  
- close the Installation Manager  

##4. Compile the libexif  

###4.1 Prepare the build  
Go to folder C:\MinGW\msys\1.0\etc\ and rename the file "fstab.sample" into "fstab"  
Edit the file fstab (right-click / Edit with Notepad++)  
On the line 16, replace "c:/mingw" by "C:/Qt/Qt5.4.0/Tools/mingw491_32"  
Save the file  

###4.2 Build  
Extract the content of libexif-0.6.21.zip at the root of the repository.  
You will then find the build procedure in pics-dl/libexif-0.6.21/README-Win32.txt  
Open MSYS/MinGW by launching:  

    C:\MinGW\msys\1.0\msys.bat  
	
In the terminal, write these commands:  

    cd /e/.../pics-dl/libexif/        # /e/ is the drive letter. Replace ... by the actual location of the git repository  
    ./get_libexif.sh
	
If successful, this file should be created: */pics-dl/libexif-0.6.21/libexif/.libs/libexif-12.dll  



#How to compile on Linux

Open a terminal and go where you wish to clone the project, then enter these commands, one by one:  
**Note: The 3rd command may last a few hours, depending on your internet connection and processing power.**

    sudo apt-get install -y git;
    git clone https://github.com/clement-nardi/pics-dl.git;
    cd pics-dl/Qt/; sudo ./linux_dependencies.sh; cd ..;
    qtcreator PicsDL/PicsDL.pro &

When QT Creator opens, **do not click on Configure Project** immediately, instead:
- go to menu Tools/Options, tab "Build & Run", sub-tab "Qt Versions"
- click on "Add..." and browse to /usr/local/Qt-5.4.0/bin/qmake
- click on "Apply", switch to the sub-tab "Kits", and click on "Add"
  - edit the name of the new kit and set it to "Qt-5.4.0 Static"
  - select Qt Version "Qt 5.4.0 (Qt-5.4.0)"
- click on OK

You should then be in the tab "Projects", sub-tab "Configure Project":
- un-select "Desktop"
- select "Qt-5.4.0 Static"
- click on "Configure Project"

You can now click on the green triangle to compile the project!


#How to compile on OS X


