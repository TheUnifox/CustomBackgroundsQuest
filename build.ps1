Param(
    [Parameter(Mandatory=$false)]
    [Switch] $clean,

    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help -eq $true) {
    echo "`"Build`" - Copiles your mod into a `".so`" or a `".a`" library"
    echo "`n-- Arguments --`n"

    echo "-Clean `t`t Deletes the `"build`" folder, so that the entire library is rebuilt"

    exit
}

# if user specified clean, remove all build files
if ($clean.IsPresent)
{
    if (Test-Path -Path "build")
    {
        remove-item build -R
    }
}


if (($clean.IsPresent) -or (-not (Test-Path -Path "build")))
{
    $out = new-item -Path build -ItemType Directory
} 

#to put into compile_commands maybe (nvm didnt work, keeping anyways):  -IC:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.32.31326/include -IZ:/HeckQuest/android-ndk-r23-windows/android-ndk-r23/toolchains/llvm/prebuilt/linux-x86_64 -IZ:/HeckQuest/android-ndk-r23-windows/android-ndk-r23/sources/cxx-stl/llvm-libc++/include -IZ:/HeckQuest/android-ndk-r23-windows/android-ndk-r23/sources/cxx-stl/llvm-libc++abi/include

& cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo"
& cmake --build .