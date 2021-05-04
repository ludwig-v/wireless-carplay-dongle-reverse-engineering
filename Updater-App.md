# Dongle Update App

These are some notes about the web app hosted at 192.168.50.2 by the dongle.

## Findings

JavaScript is compiled using Webpack (no surprise). From `2020.10.28` to `2021.03.06`, it seems like the developers have added a few modern ES features like `let` and `fetch`.

Bundle seems to include CSS and some loading code at the very end. Weird since the CSS is also available as a separate file. No frameworks or libraries that seem obvious to me.

There is a server component to this in the `cgi-bin/` folder, but those files are compiled binaries and I don't know what to make of them. That said, a lot of the data seems to map to various scripts and binaries found in the firmware, so it might not be too hard to replicate it.

Dongle settings are saved as files under `/etc`, such as `BackgroundMode`, `MediaQuality`

Check the QuickDiff link for diffs: https://quickdiff.net/?unique_id=DA549395-6EC9-4B7D-1A26-94CFA732C7FA

### Internationalization

`_lang_js__WEBPACK_IMPORTED_MODULE_1__` is for translation strings. All the strings are in a chunk early on in the file; Webpack seems to have uglified them to single and double letter variable names.

Each string is a tuple of format:

```typescript
type Translation = [ chinese: string, taiwanese: string, english: string ];
```

### API requests

Frontend makes a bunch of requests to the device's HTTP server:

`/server.cgi?cmd=getversion`. Returns an object of the following form:

```
Request URL: http://192.168.50.2/cgi-bin/server.cgi?cmd=getversion
Request Method: GET
Status Code: 200 OK
Remote Address: 192.168.50.2:80
Referrer Policy: strict-origin-when-cross-origin

{
  "version":"2021.03.09.0001",
  "type":"PU",
  "boxMac":"00:e0:4c:6b:57:4a",
  "fileName":"U2W_AUTOKIT_Update.img"
}
```

```
interface GetVersionResponse {
  type: string;
  boxMac: string;
  version: string;
  fileName: string;
}
```

`/server.cgi?cmd=is_box_activated`. Returns an object of the following form:

```
Request URL: http://192.168.50.2/cgi-bin/server.cgi?cmd=is_box_activated
Request Method: GET
Status Code: 200 OK
Remote Address: 192.168.50.2:80
Referrer Policy: strict-origin-when-cross-origin

{
  "uuid":"5988f525123199d7260c4c6190e3b999",
  "old_uuid":"5988f525123199d7260c4c6190e3b999",
  "manufacturer":"GuanSheng",
  "isActivated":1,
  "code":"lV6vo7iFXs2aCv4Ka6SY25CkwSdeXY1D",
  "boxType":"PU",
  "carManufacturer":"Mazda",
  "carModel":"MAZDA3",
  "carOemName":"Mazda",
  "carResolution":"1280x480",
  "productType":"U2W"
}
```

```
interface IsBoxActivatedResponse {
  boxType: string;
  carManufacturer: string;
  carModel: string;
  carOemName: string;
  carResolution: string;
  code: string;
  isActivated: 0 | 1;
  manufacturer: string:
  old_uuid: string:
  productType: string;
  uuid: string;
}
```

`/server.cgi?cmd=get_settings`. Returns an object of the following form:

```
Request URL: http://192.168.50.2/cgi-bin/server.cgi?cmd=get_settings
Request Method: GET
Status Code: 200 OK
Remote Address: 192.168.50.2:80
Referrer Policy: strict-origin-when-cross-origin

{
  "SyncMode":0,
  "SoundQuality":1,
  "MediaDelay":1000,
  "UMode":0,
  "SaveLog":1,
  "ImageMode":0,
  "EchoDelay":320,
  "StartDelay":0,
  "FrameRate":0,
  "AutoConn":1,
  "BgMode":1,
  "GPS":1,
  "DisplaySize":0
}
```

```
interface GetSettingsResponse {
  AutoConn: 0 | 1;
  BgMode: 0 | 1;
  DisplaySize: 0 | 1;
  EchoDelay: number;
  FrameRate: number;
  GPS: 0 | 1;
  ImageMode: 0 | 1;
  MediaDelay: number;
  SaveLog: 0 | 1;
  SoundQuality: 0 | 1; // CD | DVD
  StartDelay: number;
  SyncMode: 0 | 1;
  UMode: 0 | 1;
}
```

It then makes a couple of requests to paplink.cn to check for updates (won't work on custom firmware):

```
Request URL: http://www.paplink.cn/server.php?action=checkBoxUpdateByCustomerBoxType&lang=2&CustomerType=P&BoxType=U&uuid=&fileName=U2W_AUTOKIT_Update.img&curVer=2021.03.09.0001
Referrer Policy: strict-origin-when-cross-origin

{
  "filePath": "",
  "fileSize": 0,
  "fileName": "U2W_Update.img",
  "version": "",
  "updateTime": "",
  "updateLog": ""
}

Request URL: http://www.paplink.cn/server.php?action=checkBoxUpdateByCustomerBoxType&lang=2&CustomerType=P&BoxType=U&uuid=5988f525123199d7260c4c6190e3b999&fileName=U2W_AUTOKIT_Update.img&curVer=2021.03.09.0001
Referrer Policy: strict-origin-when-cross-origin

{
  "filePath": "/mnt/downloads/2021-03-21/U2W_AUTOKIT_Update_2021.03.06.1355.img",
  "fileSize": "10277918",
  "fileName": "U2W_AUTOKIT_Update.img",
  "version": "2021.03.06.1355",
  "updateTime": "2021-04-16 13:58:34",
  "updateLog": "Fix some bugs\nFix disconnect problem on MAZDA"
}
```

Also, there seems to be a unique URL for each setting:

```
Request URL: http://192.168.50.2/cgi-bin/server.cgi?cmd=set_sound_quality&value=0
Request Method: GET
Status Code: 200 OK
Remote Address: 192.168.50.2:80
Referrer Policy: strict-origin-when-cross-origin

0

Request URL: http://192.168.50.2/cgi-bin/server.cgi?cmd=set_sound_quality&value=1
Request Method: GET
Status Code: 200 OK
Remote Address: 192.168.50.2:80
Referrer Policy: strict-origin-when-cross-origin

0

Request URL: http://192.168.50.2/cgi-bin/server.cgi?cmd=set_sync_mode&value=0
Request Method: GET
Status Code: 200 OK
Remote Address: 192.168.50.2:80
Referrer Policy: strict-origin-when-cross-origin

0
```

