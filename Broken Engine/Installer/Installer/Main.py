from ScanDirs import *
from shutil import copyfile


def CreateFinalProduct():
    wxs.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    wxs.write('<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi" xmlns:util="http://schemas.microsoft.com/wix/UtilExtension">\n')
    # product and package
    wxs.write('\t<Product Id="{}" Name="{}" Language="1033" Version="{}" Manufacturer="{}" UpgradeCode="{}">'
              .format(product['id'], product['name'],product['version'], product['manufacturer'],product["upgradeCode"]))
    wxs.write('\t\t<Package Id="{}" InstallerVersion="200" Compressed="yes" InstallScope="perMachine" Description="{}" Manufacturer="{}" />\n'
              .format(package["id"], package["description"], package["manufacturer"]))
    # first icon is for the add and remove programs, second is for the shortcuts
    # Properties
    wxs.write('\t\t<Property Id="ARPPRODUCTICON">{}</Property>\n'.format(icon['internal_id']))
    wxs.write('\t\t<Property Id="WIXUI_INSTALLDIR" Value="INSTALLDIR" />\n')
    # Launch at finish checkbox
    wxs.write('\t\t<Property Id="WIXUI_EXITDIALOGOPTIONALCHECKBOXTEXT" Value="Launch {}" />\n'.format(product['name']))
    wxs.write('\t\t<Property Id="WixShellExecTarget" Value="[#{}]" />\n'.format(main_exe['id']))
    wxs.write('\t\t<CustomAction Id="LaunchApplication" BinaryKey="WixCA" DllEntry="WixShellExec" Impersonate="yes" />\n')

    # UI
    wxs.write('\t\t<UI>\n')
    wxs.write('\t\t\t<UIRef Id="WixUI_InstallDir" />\n')
    wxs.write('\t\t\t<UIRef Id="WixUI_ErrorProgressText" />\n')
    wxs.write('\t\t\t<Publish Dialog="ExitDialog" Control="Finish" Event="DoAction" Value="LaunchApplication">WIXUI_EXITDIALOGOPTIONALCHECKBOX = 1 and NOT Installed</Publish>\n')
    wxs.write('\t\t</UI>\n')
    # License
    wxs.write('\t\t<WixVariable Id = "WixUILicenseRtf" Value = "{}" />\n'.format(ui['license']))
    # Images
    if custom_ui:
        wxs.write('\t\t<WixVariable Id="WixUIBannerBmp" Value = "{}" />\n'.format(ui['topBanner']))
        wxs.write('\t\t<WixVariable Id="WixUIDialogBmp" Value = "{}" />\n'.format(ui['dialogBg']))

    # Icon both program files and shortcuts
    wxs.write('\t\t<Icon Id="{}" SourceFile="{}" />\n'.format(icon['internal_id'], icon['src']))
    wxs.write('\t\t<Icon Id="{}" SourceFile="{}" />\n'.format(icon['id'], icon['src']))

    wxs.write('\t\t<MajorUpgrade DowngradeErrorMessage="{}" />\n'.format(product['downgrade']))
    wxs.write('\t\t<MediaTemplate EmbedCab="yes" />\n') # change to "no" if you want a separated .cab file along with the installer

    # Components Reference
    wxs.write('\t\t<Feature Id="ProductFeature" Title="ProductFeatures" Display="expand" Level="1" ConfigurableDirectory="INSTALLDIR">\n')
    wxs.write('\t\t\t<ComponentGroupRef Id="ProductComponents"/>\n')

    # Here is where "componentsReference.txt" needs to be placed
    for line in componentsReference.readlines():
        wxs.write('\t\t\t' + line)

    wxs.write('\t\t</Feature>\n')
    wxs.write('\t</Product>\n')

    # Directories
    wxs.write('\t<Fragment>\n')
    wxs.write('\t\t<Directory Id="TARGETDIR" Name="SourceDir">\n')
    wxs.write('\t\t\t<Directory Id="ProgramFilesFolder">\n')
    wxs.write('\t\t\t\t<Directory Id="{}" Name="{}">\n'.format(package['manufacturer'], package['manufacturer']))
    wxs.write('\t\t\t\t\t<Directory Id="INSTALLDIR" Name="{}">\n'.format(product['name']))

    # Here is where "directories.txt" needs to be placed, we are inside the install folder
    for line in directories.readlines():
        wxs.write('\t\t\t\t\t'+line)

    wxs.write('\t\t\t\t\t</Directory>\n')
    wxs.write('\t\t\t\t</Directory>\n')
    wxs.write('\t\t\t</Directory>\n')

    # Programs menu folder
    wxs.write('\t\t\t<Directory Id="ProgramMenuFolder" Name="Programs">\n')
    wxs.write('\t\t\t\t<Directory Id="ProgramMenuDir" Name="{}"/>\n'.format(product['name']))
    wxs.write('\t\t\t</Directory>\n')

    # Desktop folder
    wxs.write('\t\t\t<Directory Id="DesktopFolder" Name="Desktop" />\n')
    wxs.write('\t\t</Directory>\n')
    wxs.write('\t</Fragment>\n')

    # Components
    wxs.write('\t<Fragment>\n')
    # Program menu components
    wxs.write('\t\t<ComponentGroup Id="ProductComponents" Directory="INSTALLDIR">\n')
    wxs.write('\t\t\t<Component Id="ProgramMenuDir" Guid="*">\n')
    wxs.write('\t\t\t\t<RemoveFolder Id="RemoveProgramMenuDir" Directory="ProgramMenuDir" On="uninstall" />\n')
    # Registy entries
    wxs.write('\t\t\t\t<RegistryValue Root="HKCU" Key="Software\{}\{}" Type="string" Value="" KeyPath="yes" />\n'.format(product['manufacturer'],product['name']))
    wxs.write('\t\t\t</Component>\n')
    wxs.write('\t\t</ComponentGroup>\n')

    # Here is where "components.txt" needs to be placed
    for line in components.readlines():
        wxs.write('\t\t' + line)

    wxs.write('\t</Fragment>\n')
    wxs.write('</Wix>\n')


if __name__ == '__main__':

    print('-Parsing root directory-----------------------------')
    # Creating the necessary files just for the root directory
    CreateRoot()

    # Rest of the folders and files
    if not simplified_version:
        print('-Parsing all directories-----------------------------')
        Recursive(base_path,0)
    else:
        print('-Excluding all directories (disable simplified_version to include the whole directory)-----------------------------')

    # Reseting the cursor to read from files
    directories.seek(0)
    directories = open(os.path.join(internal_path, "directories.txt"), 'r')

    components.seek(0)
    components = open(os.path.join(internal_path, "components.txt"), 'r')

    componentsReference.seek(0)
    componentsReference = open(os.path.join(internal_path, "componentsReference.txt"), 'r')

    print('-Creating XML file-----------------------------')
    CreateFinalProduct()

    Close()

    if create_installer:
        installer_name = product['name'].replace(' ','') + '_installer'
        print('-Creating {}.msi-----------------------------'.format(installer_name))
        print()
        # candle.exe
        os.system('.\Wixbinaries\candle.exe -ext WixUtilExtension -ext WixUiExtension '
                  + os.path.join(internal_path,'product.wxs') +' -o ' + os.path.join(internal_path,'product.wixobj'))
        # light.exe
        os.system('.\Wixbinaries\light.exe ' + os.path.join(internal_path,'product.wixobj')+' -ext WixUtilExtension -ext WixUiExtension -o {}'.format(installer_name))

        print()
        print('-Installer created at ' + os.getcwd() + installer_name + '.msi-----------------------------')






