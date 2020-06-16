import os

create_installer = True  # if false only create the product.wxs
simplified_version = False  # if true only parses the root directory of the base_path
desktop_shortcut = True  # I think it's self explanatory
menu_shortcut = True  # Windows start menu shortcut [manufacturer]/[name]/[main_exe.name]
internet_shortcut = True  # I think it's self explanatory
custom_ui = True  # I think it's self explanatory

base_path = "D:/Users/William/Desktop/The Witcher Ties of Destiny"  # folder to replicate
internal_path = "Internal"  # no need to edit this unless you want debug files somewhere else

excluded_files = ['topbanner.bmp', 'dialogBackground.bmp', 'installericon.ico', 'TheWitcherTODicopng.png']  # I think it's self explanatory

# product
product = {
    'id': '69D9A983-2BB9-41C5-879C-944AE175DA2A',
    # id - this have to be saved and constant for windows to detect the app, must be the same or windows
    # cannot tell if it is the same product, must remain constant between upgrades
    'name': 'The Witcher Ties of destiny',  # name of most folders and reference to the product
    'version': '1.0',  # I think it's self explanatory
    'manufacturer': 'BrokenGem',  # I think it's self explanatory
    'upgradeCode': '70EDF160-2665-4CC2-B0C2-3AAF6CC3ACFC',  # this needs to be changed when upgrading, I think
    'downgrade': 'A newer version of The Witcher Ties of Destiny is already installed'
    # downgrade error message to be prompt
}

# package
package = {
    'id': '70EA163F-F251-4155-A354-72A08AC888A8',  # id of the package you are installing, is more specific than the product id
    'description': 'The Witcher Ties of destiny Installer',  # description shown if custom installation and multiple packages, not the case but needed
    'manufacturer': product["manufacturer"]  # manufacturer of this concrete package, the whole in this case
}

# main executable
main_exe = {
    'file': 'BEG.exe',  # main application, shortcuts will point to this
    'id': 'MainExecutable.exe',  # do not edit
    'name': 'TheWitcherTOD.exe'  # final name of the executable
}

# icon
icon = {
    'id': main_exe['name'],  # must have the extension of the target
    'src': os.path.join(base_path, 'TheWitcherTOD.ico'),  # source file of the icon
    'internal_id': '_B4B5AEFE35DF'  # Do not edit this
}

# UI - these settings will be ignored if custom_ui is set to False
ui = {
    'license': os.path.join(base_path, 'license.rtf'),  # expected to be .rtf format
    'topBanner': os.path.join(base_path, 'topbanner.bmp'),  # 493x58 .bmp
    'dialogBg': os.path.join(base_path, 'dialogBackground.bmp'),  # 493x312 .bmp
}

# internet shortcuts - these settings will be ignored if internet_shortcut is set to False
internet = {
    'url': "https://www.tiesofdestiny.com",  # I think it's self explanatory
    'name': 'BrokenGem',  # I think it's self explanatory
}

# Do not edit below this line -----------------
# internal files, do not edit
directories = open(os.path.join(internal_path, "directories.txt"), "w")
components = open(os.path.join(internal_path, "components.txt"), "w")
componentsReference = open(os.path.join(internal_path, "componentsReference.txt"), "w")
wxs = open(os.path.join(internal_path, "product.wxs"), 'w')


def Close():
    global directories, components, componentsReference, wxs
    directories.close()
    components.close()
    componentsReference.close()
    wxs.close()
