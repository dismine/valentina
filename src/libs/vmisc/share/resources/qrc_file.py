import os

def traverse_directory(root_dir, relative_dir="", file_list=[]):
    current_path = os.path.join(root_dir, relative_dir)
    for entry in os.listdir(current_path):
        entry_path = os.path.join(current_path, entry)
        relative_path = os.path.join(relative_dir, entry)
        if os.path.isdir(entry_path):
            traverse_directory(root_dir, relative_path, file_list)
        else:
            file_list.append(relative_path)

def generate_qrc_file(icon_theme_dir, root_folder, qrc_filename):
    file_list = []
    traverse_directory(icon_theme_dir, "", file_list)

    qrc_content = f'<RCC>\n    <qresource prefix="/">\n'
    for file_path in file_list:
        qrc_content += f'        <file>{root_folder}{file_path}</file>\n'
    qrc_content += '    </qresource>\n</RCC>'

    with open(qrc_filename, 'w', encoding='utf-8') as qrc_file:
        qrc_file.write(qrc_content)

if __name__ == "__main__":
    root_folder = "icons/La-Sierra-Dark/"
    icon_theme_directory = "/home/dismine/CAD/Valentina_0.7.x/valentina/src/libs/vmisc/share/resources/icons/La-Sierra-Dark"
    qrc_file_name = "mac_dark_theme.qrc"

    generate_qrc_file(icon_theme_directory, root_folder, qrc_file_name)

