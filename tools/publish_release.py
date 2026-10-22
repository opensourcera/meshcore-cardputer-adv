"""Create the GitHub release for a tag and upload the files in out/.
usage: python tools/publish_release.py <tag>
Uses the GitHub token stored in the git credential manager (git credential fill)."""
import json, os, subprocess, sys, urllib.request

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
REPO = 'opensourcera/meshcore-cardputer-adv'
tag = sys.argv[1]

cred = subprocess.run(['git', 'credential', 'fill'], input='protocol=https\nhost=github.com\n\n', capture_output=True, text=True).stdout
token = next(l.split('=', 1)[1] for l in cred.splitlines() if l.startswith('password='))
hdr = {'Authorization': 'token ' + token, 'Accept': 'application/vnd.github+json'}

body = """MeshCore companion firmware for the M5Stack Cardputer ADV with Cap LoRa-1262, with a **Font size** setting.

**What is new**
- Tools > Signal tester: pick a repeater, optionally one to go through, and get five traces with the SNR there and back.
- Settings > Font size: 12 (original), 14, 16 or 18 px. Live preview while choosing, `Enter` saves, `Esc` reverts.
- Menus, lists, chat history and the message line follow the selected font.
- Settings, contacts, channels and chat history stay on the SD card and are kept across updates.

**Install**
Flash the `-merged.bin` file at address `0x0` with https://meshcore.io/flasher or:

```
esptool --chip esp32s3 --port COMx write-flash 0x0 cardputer_adv_companion_radio_ble-v1.17.1-%s-merged.bin
```

The plain `.bin` is the application only, for `0x10000` on a device that already runs this firmware.

Based on MultiMote/meshcore-cardputer-adv 2026.10.1 and MeshCore 1.17.1. See the README for the preview image and details.
""" % tag

req = urllib.request.Request('https://api.github.com/repos/%s/releases' % REPO, method='POST',
                             headers=dict(hdr, **{'Content-Type': 'application/json'}),
                             data=json.dumps({'tag_name': tag, 'name': 'Firmware %s: font size setting and signal tester' % tag,
                                              'body': body, 'draft': False, 'prerelease': False}).encode())
rel = json.load(urllib.request.urlopen(req))
print('release:', rel['html_url'])

out = os.path.join(ROOT, 'out')
for fn in sorted(os.listdir(out)):
    if tag not in fn:
        continue
    data = open(os.path.join(out, fn), 'rb').read()
    up = rel['upload_url'].split('{')[0] + '?name=' + fn
    r = urllib.request.Request(up, data=data, method='POST', headers=dict(hdr, **{'Content-Type': 'application/octet-stream'}))
    a = json.load(urllib.request.urlopen(r))
    print('uploaded:', a['name'], a['size'], 'bytes')
