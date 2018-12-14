print('bootstrapping dependency installer ...')

def bootstrap():
    import subprocess
    import sys
    def install(package):
        subprocess.call([sys.executable, "-m", "pip", "install", package])
    install('pygit2')
    print('installing PythonGit')


# -----------------------------------------------------------------------------
import subprocess
import os.path
import os
import shutil

def git_clone(git_url, local_dir, tag = None):
    print('git-cloning', local_dir, '<-', git_url)
    if os.path.exists(local_dir):
        print('    skipped - target', local_dir, 'exists already')
        return
    import pygit2
    repo = pygit2.clone_repository(git_url, local_dir)
    if tag is not None:
        repo.checkout(tag)
    print('done.')

def download_file(url, filename):
    print('downloading', filename, '<-', url,'...')
    if os.path.exists(filename):
        print('    skipped - target', filename, 'exists already')
        return
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    import urllib.request
    urllib.request.urlretrieve(url, filename)
    print('done.')

def unzip(filename, local_dir):
    print('unzipping', local_dir, '<-', filename,'...')
    if os.path.exists(local_dir):
        print('    skipped - target', local_dir, 'exists already')
        return
    import zipfile
    with zipfile.ZipFile(filename, 'r') as zip_ref:
        zip_ref.extractall(local_dir)
    print('done.')

def start_in_folder(what, where, if_not_exists, args=[]):
    print('starting', what, 'from working directory', where,'...')
    owd = os.getcwd()
    try:
        os.chdir(where)
        if os.path.exists(if_not_exists):
            print('    skipped - indicated as done already (i.e. ', if_not_exists, 'exists)')
            return
        os.system(what + ' ' + ' '.join(args))
        print('done.')
    finally:
        os.chdir(owd)

bootstrap()

