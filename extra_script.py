Import("env")
from shutil import copy
import subprocess
import gzip
import io
import os
import shutil

def write_gzip(file_path):
    in_file = open(file_path, 'rb')
    out_file = gzip.open(file_path + ".gz", 'wb')
    for line in in_file.readlines():
        out_file.write(line)
    in_file.close()
    out_file.close()

def compress_html(folder_path, file_name):
    write_gzip(folder_path + file_name)
    compressedHtml = folder_path + file_name + '.gz'
    finalHtml = './data/' + file_name + '.gz'
    print("Copying %s to %s".format(compressedHtml, finalHtml))
    copyfile(compressedHtml, finalHtml)


def pre_build_fs_hook(source, target, env):
    print("Running pre_build_fs_hook")
    print("Running gulp")

    # Terrible hack to make sure that the correct node version is used
    my_env = os.environ.copy()
    my_env["PATH"] = f"{my_env['HOME']}/.nvm/versions/node/v18.17.1/bin:{my_env['PATH']}"

    subprocess.run('cd web && npm run build', shell=True, env=my_env)


    source_folder = r"./web/build/"
    destination_folder = r"./data/"

    # fetch all files
    for file_name in os.listdir(source_folder):
        print(file_name)
        # construct full file path
        source = source_folder + file_name
        destination = destination_folder + file_name
        # copy only files
        if os.path.isfile(source):
            copy(source, destination)
            print('copied', file_name)


    # compress_html('./web/dist/', 'index.html')

    print("Successfully completed before_fs_upload hook")
    # do some actions

env.AddPreAction("$BUILD_DIR/littlefs.bin", pre_build_fs_hook)
