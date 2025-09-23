# Debugging using Reverse RDP with ngrok

*Enable RDP and connect to the Windows GitHub Actions runner using ngrok, for deeper debugging of workflows.*

---

## Features

- Enable Remote Desktop (RDP) on GitHub Actions’ Windows runners.  
- Set a temporary local administrator user (`runneradmin`) with configurable password.  
- Tunnel RDP traffic through ngrok, so you can connect from your machine.  
- Add a blocking “breakpoint” using a file (`Delete-This-File-To-Continue.txt`) on the Desktop to pause workflow until you finish debugging.

---

## Usage

### 1. Get your ngrok auth token

Sign up or log in to [ngrok](https://ngrok.com) → get your auth token from their dashboard.

### 2. Store the token as a GitHub secret

In your repository’s **Settings → Secrets and variables → Actions**, create a secret:

- **Name:** `NGROK_AUTH_TOKEN`
- **Value:** *Your ngrok auth token*

### 3. Add the action to your workflow

Here’s an example step in your workflow:

```yaml
jobs:
  debug:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4

      - name: Reverse RDP via ngrok
        if: failure() # only run when earlier steps failed
        uses: ./.github/actions/reverse-rdp-windows
        with:
          ngrok-token: ${{ secrets.NGROK_AUTH_TOKEN }}
          password: ${{ secrets.RDP_PASSWORD }}
```          

### 4. Connect using RDP client

1. Visit the ngrok Dashboard → check under **Universal Gateway** → **Endpoints & Traffic Policy** for the host and port your tunnel is exposing.

2. Use an RDP client of choice (for example, **Remmina**, **Microsoft Remote Desktop**, etc.), with credentials:

 - **Host**: as shown by ngrok (e.g. `0.tcp.ngrok.io:<port>`)

 - **Username**: `runneradmin`

 - **Password**: whatever you set in the workflow (`password` input)

### 5. Finish debugging

Once you're done with your investigation:

* *Delete* the file named `Delete-This-File-To-Continue.txt` on the Desktop of the Windows runner. That tells the workflow you’re done, so it can continue/clean up.

### Optional settings

* `ngrok_download_url`: lets you override the default ngrok Windows (amd64) ZIP download URL.  
  By default it uses:  
  `https://bin.equinox.io/c/bNyj1mQVY4c/ngrok-v3-stable-windows-amd64.zip`  
  You can point this to a different ngrok build if needed (for example, a mirror or custom version).

* You can run this action conditionally (for example, only on failure, or only when manually triggered) using if: in your workflow yaml.

### Limitations

* Runner jobs have time limits (often ~6 hours). You have only so much time to perform debugging.

* Security: be mindful when exposing RDP over ngrok; ensure the password is strong and kept secret.
