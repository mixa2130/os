import subprocess

from pathlib import Path

if __name__ == '__main__':
    code = subprocess.call(['gcc', 'main.c', '-o', 'check'])
        
    if code != 0:
        print('Something went wrong! Maybe you have to update your gcc')
    else:
        ls_code = subprocess.call(['./check', Path.cwd()])
        
        if ls_code != 0:
            print("Write to the developer, that doesn't work")
            
