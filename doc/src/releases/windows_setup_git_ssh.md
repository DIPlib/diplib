# Setup GitHub SSH authentication

Pushing to and pulling from GitHub requires authentication. This is done with an ssh-keypair without a password to speed up the process. First the ssh-keypair is created and shared with GitHub. Secondly Windows is setup to store the private key in memory after each login.

## Store new ssh-key in GitHub

1. open `Git Bash`
1. enter the following command to create an ssh-keypair without passphrase:

   ```bash
   ssh-key -f ~/.ssh/github -N ""
   ```

1. copy the contents of the **public** key `~/.ssh/github.pub`
1. open GitHub website and navigate to `Settings` in the user-menu in the upper-right
1. select on the left `SSH and GPG keys` and press the <kbd>New SSH key</kbd> button
1. paste the public key in the `Key` field and press the <kbd>Add SSH key</kbd> button

## Save ssh-key in Windows

The private part of the ssh-key is stored in memory at login in the ssh-agent.

1. open `Services`
1. right-click on `OpenSSH Authentication Agent` and select `Properties`
1. set `Startup type` to `Automatic` and click <kbd>Apply</kbd>
1. click on the <kbd>Start</kbd> button close window with <kbd>OK</kbd>
1. create `ssh_add_github.bat` with the following content:

   ```bat
   ssh-add.exe "C:\Users\<username>\.ssh\github"
   ```

1. store the `ssh_add_github.bat` file in the user startup directory: `%appdata%\Microsoft\Windows\Start Menu\Programs\Startup`
1. open `Environment Variables`
1. add User variable `GIT_SSH` with the value `C:\Windows\System32\OpenSSH\ssh.exe`
1. logout and login again
