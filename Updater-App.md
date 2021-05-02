# Dongle Update App

These are some notes about the web app hosted at 192.168.50.2 by the dongle.

## Running on a Mac

Plug the dongle into your Mac. Also connect an iPhone using USB and connect to its Personal Hotspot.

Go to the Mac's System Preferences > Network > iPhone USB and uncheck 'Disable unless needed'.

Connect to the dongle's WiFi and open 192.168.50.2; not everything will work but its a decent start.

## Findings

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

It seems to make a couple of requests to paplink.cn:

```
Request URL: http://www.paplink.cn/server.php?action=checkBoxUpdateByCustomerBoxType&lang=2&CustomerType=P&BoxType=U&uuid=&fileName=U2W_AUTOKIT_Update.img&curVer=2021.03.09.0001
Referrer Policy: strict-origin-when-cross-origin

Request URL: http://www.paplink.cn/server.php?action=checkBoxUpdateByCustomerBoxType&lang=2&CustomerType=P&BoxType=U&uuid=5988f525123199d7260c4c6190e3b999&fileName=U2W_AUTOKIT_Update.img&curVer=2021.03.09.0001
Referrer Policy: strict-origin-when-cross-origin
```

There seems to be a unique URL for each setting:

```
Request URL: http://192.168.50.2/cgi-bin/server.cgi?cmd=set_sound_quality&value=0
Request Method: GET
Status Code: 200 OK
Remote Address: 192.168.50.2:80
Referrer Policy: strict-origin-when-cross-origin

Request URL: http://192.168.50.2/cgi-bin/server.cgi?cmd=set_sync_mode&value=0
Request Method: GET
Status Code: 200 OK
Remote Address: 192.168.50.2:80
Referrer Policy: strict-origin-when-cross-origin
```

