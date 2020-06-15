from Settings import *


def OpenDir(dir, name, level):
    # print("Opening " + dir.replace(base_path,''))
    id = dir.replace(base_path, '').replace(' ', '_').replace("\\", '_')
    tabs = '\t' * level
    folder_name = 'Folder' + id
    directories.write(tabs + '<Directory Id="{}" Name="{}" >\n'.format(folder_name, name))

    CreateFiles(id, folder_name, dir)


def CloseDir(level):
    if level == -1:
        return
    # print("Closing " + dir.replace(base_path ,''))
    tabs = '\t' * level
    directories.write(tabs + '</Directory>\n')


# id of the current path, concatenating 'Components' to write it in ComponentGroupRef
# dir of the current path, the Directory reference to the folder already created
# path is the os path to search in files
def CreateFiles(id, dir, path):
    componentsGroupId = 'Components' + id
    componentsReference.write('<ComponentGroupRef Id="{}" />\n'.format(componentsGroupId))
    components.write('<ComponentGroup Id="{}" Directory="{}">\n'.format(componentsGroupId, dir))

    for file in os.listdir(path):
        if file in excluded_files:
            continue

        file_path = os.path.join(path, file)
        if os.path.isfile(file_path):
            # Create file in components file
            compId = id + file.replace(' ', '_').replace('-', '_').replace('(', '_').replace(')', '_').replace('#', '_')
            if len(compId) >= 71:  # clamping for warnings
                compId = compId[-71:]

            fileId = 'File' + compId
            if len(fileId) >= 71:
                fileId = 'File' + compId[4:]

            components.write('\t<Component Id="{}" Guid="*">\n'.format(compId))
            components.write(
                '\t\t<File Id="{}" Name="{}" DiskId="1" Source="{}" KeyPath="yes" />\n'.format(fileId, file, file_path))
            components.write('\t</Component>\n')

    components.write('</ComponentGroup>\n\n')


def CreateRoot():
    componentsReference.write('<ComponentGroupRef Id="RootComponents" />\n')

    components.write('<ComponentGroup Id="RootComponents" Directory="INSTALLDIR">\n')

    # Create files of the main directory, rest is done recursively
    # main executable expected to be inside this folder
    for file in os.listdir(base_path):
        if file in excluded_files:
            continue
        file_path = os.path.join(base_path, file)

        if file == main_exe['file']:
            print('-Found main exe "{}"-----------------------------'.format(file))

            components.write('\t<Component Id="MainExecutable" Guid="*">\n')
            components.write(
                '\t\t<File Id="{}" Name="{}" DiskId="1" Source="{}" KeyPath="yes" >\n'.format(main_exe['id'],
                                                                                              main_exe['name'],
                                                                                              file_path))
            if menu_shortcut:
                components.write(
                    '\t\t\t<Shortcut Id="startMenuShortcut" Directory="ProgramMenuDir" Name="{}" WorkingDirectory="INSTALLDIR" Icon="{}" IconIndex="0" Advertise="yes" />\n'.format(
                        product['name'], icon['id']))
            if desktop_shortcut:
                components.write(
                    '\t\t\t<Shortcut Id="desktopShortcut" Directory="DesktopFolder" Name="{}" WorkingDirectory="INSTALLDIR" Icon="{}" IconIndex="0" Advertise="yes" />\n'.format(
                        product['name'], icon['id']))

            components.write('\t\t</File>\n')
            if internet_shortcut:
                components.write(
                    '\t\t\t<util:InternetShortcut Id="OnlineDocumentationShortcut" Name="{}" Target="{}"/>\n'.format(
                        internet['name'], internet['url']))

            components.write('\t</Component>\n')

        elif os.path.isfile(file_path):
            compId = 'root' + file.replace(' ', '_').replace('-', '_').replace('(', '_').replace(')', '_').replace('#',
                                                                                                                   '_')
            fileId = 'File' + compId
            components.write('\t<Component Id="{}" Guid="*">\n'.format(compId))
            components.write(
                '\t\t<File Id="{}" Name="{}" DiskId="1" Source="{}" KeyPath="yes" />\n'.format(fileId, file, file_path))
            components.write('\t</Component>\n')

    components.write('</ComponentGroup>\n\n')


def Recursive(path, level):
    for dir in os.listdir(path):
        p = os.path.join(path, dir)

        # print(p)
        if os.path.isdir(p):
            OpenDir(p, dir, level)
            Recursive(p, level + 1)

    CloseDir(level - 1)
