Import("env")
from shutil import copyfile
import subprocess
import gzip
import io

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
    p = subprocess.Popen('cd web && npm run build', shell=True, stdout=subprocess.PIPE)
    stdout, stderr = p.communicate()
    #TODO: Make this output realtime
    for line in stdout.splitlines():
        line = line.rstrip()
        print(line)

    compress_html('./web/dist/', 'index.html')

    print("Successfully completed before_fs_upload hook")
    # do some actions

env.AddPreAction("$BUILD_DIR/littlefs.bin", pre_build_fs_hook)
