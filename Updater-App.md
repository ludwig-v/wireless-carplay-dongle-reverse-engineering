# Dongle Update App

These are some notes about the web app hosted at 192.168.50.2 by the dongle.

## Settings

Settings stored as riddle.conf file at `/etc/riddle.conf`.

Using `riddleConf --info` default values could be extracted.

| Name | Internal name | Default Value | Possible values | Comment |
|--|--|--|--|--|
| CustomFrameRate | fps | 0 | 0-60 | When the box is connected to the mobile phone to synchronize the frame rate of the screen refresh, this item can be set if the connection screen is delayed more. (If the screen is obviously delayed, please set the frame rate to 20) |
| HudGPSSwitch | gps | 0 | 0-1 | Enables GPS via CarPlay. When enabled, the adapter passes the GPS coordinates received from the head unit via the car's GPS antenna in order to improve positioning, especially when the iPhone is hidden away, lacking a clear line-of-sight view to the satellites. This is not a mandatory feature for wired CarPlay, so it may not be supported by cars with wired CarPlay only (probably only those with a navigation system built-in). |
| BoxConfig_UI_Lang | lang | 1033 | Defined in cfg file | LCID language code used for car screen info |
| BackgroundMode | bgMode | 0 | 0-1 | The connection interface of the box will not be displayed after opening, and this option needs to be turned on for cars that often appear blurry after connecting to the mobile phone. |
| iAP2TransMode | syncMode | 0 | 0-1 (`0` for Normal, `1` for Compatible) | It is used for the synchronization function of the instrument panel compatible with some old cars, generally does not need to be turned on. |
| BoxConfig_DelayStart | startDelay | 0 | 0-30 | Start-up delay. If you set the setting to `5`, the adapter will start the system after 5 seconds when connected to the vehicle's USB port. If a vehicle system does not recognize the adapter or the adapter only works to plug it in again, configure this setting parameter. |
| MediaLatency | mediaDelay | 1000 | 300-2000 | Adjust the delay time of the media sound, the default is 1000 milliseconds, and the useful range is 300-2000 milliseconds. The greater the delay, the less likely to be stuck. The smaller the delay, the more synchronized the music and the image. The actual situation should be adjusted appropriately, and it is not recommended to adjust if there is no abnormal situation. |
| MediaQuality | mediaSound | 1 | 0-1 (`0` for CD, `1` for DVD) | Toggle between CD and DVD sound quality. CD fixes static/playback issues with some cars. DVD has better sound quality. |
| NeedAutoConnect | autoConn | 1 | 0-1 | Turn on/off the box auto-connect phone option. |
| EchoLatency | echoDelay | 320 | 20-2000 | |
| UseBTPhone | btCall | 0 | 0-1 | Bluetooth PhoneCall |
| MicGainSwitch | micGain | 0 | 0-1 | Mic Gain |
| NeedKeyFrame | autoRefresh | 0 | 0-1 | Auto Refresh |
| DisplaySize | displaySize | 0 | 0-1 (`0` - Auto, `1` - S) | Display Style |
| AutoPlauMusic | autoPlay | 0 | 0-1 | Play music automatically after connecting the HiCar function |
| MouseMode | mouseMode | 0 | 0-1 | After it is turned on, a cursor will appear on models with a touchpad. You can use the touchpad to operate the cursor to move the cursor to select and confirm. Touch the touchpad three times to switch the sliding or dragging mode, and perform operations such as page turning. |
| UdiskMode | Udisk | 0 | 0-1 | The U-Disk mode is used for connecting to the computer. |
| VideoBitRate | bitRate | 0 | 0-20 | Modify the video bit rate mapped by the mobile phone. The smaller the bit rate setting is, the smoother the video will be. When the video is stuck and delayed, it is recommended to set the bit rate below 8 Mbps; the larger the bit rate setting, the more stable the video quality will be. The problem, you can try to set the bit rate above 8 Mbps to improve |
| VideoResolutionWidth | resolutionWidth | 1920 | 0-4096 | Modify the resolution of the mobile phone mapping image. If the display effect of the car is not good, or there is stretching deformation, you can try to adjust it to the same resolution as the central control screen. Most cars do not need to be modified. |
| VideoResolutionHeight | resolutionHeight | 720 | 0-4096 | Modify the resolution of the mobile phone mapping image. If the display effect of the car is not good, or there is stretching deformation, you can try to adjust it to the same resolution as the central control screen. Most cars do not need to be modified. |
| ScreenDPI | ScreenDPI | 0 | 0-480 | |
| KnobMode | KnobMode | 0 | 0-1 | Used for reassign steering wheel buttons. |
| NaviAudio | NaviAudio | | | |
| CallQuality | CallQuality | 1 | 0-2 | |
| AutoUpdate | autoUpdate | 1 | 0-1 | Auto update |
| WiFiChannel | wifiChannel | | | |
| CarLinkType | carLinkType | 30 | 1-30 | Device type. |
| RepeatKeyframe | RepeatKeyFrame | 0 | 0-1 | Image Initialization |
| BtAudio | BtAudio | 0 | 0-1 | Bluetooth Audio |
| MicMode | MicMode | 0 | `0` - Auto, `1` - Mode 1, `2` - Mode 2, `3` - Mode 3, `4` - Mode 4 | Different modes for microphone |
| SpsPpsMode | SpsPpsMode | 0 | `0` - Auto, `1` - Mode 1, `2` - Mode 2, `3` - Mode 3 | Video Mode |
| MediaPacketLen | MediaPacketLen | 200 | 200-40000 | |
| TtsPacketLen | TtsPacketLen | 200 | 200-40000 | |
| VrPacketLen | VrPacketLen | 200 | 200-40000 | |
| TtsVolumGain | TtsVolumGain | 0 | 0-1 | Navigation Gain |
| VrVolumGain | VrVolumGain | 0 | 0-1 | Telephone Gain |
| SendHeartBeat | heartBeat | 1 | 0-1 | Send Heartbeat |
| SendEmptyFrame | emptyFrame | 1 | 0-1 | Send Empty Frame |
| MicType | micType | 0-2 | 0 - Car, 1 - Box, 2 - Phone | Select microphone source |
| autoDisplay | autoDisplay | 0 | 0-1 | |
| USBConnectedMode | connectedMode | 0 | `0` - Mode 1, `1` - Mode 2, `2` - Mode 3 | Different connection usb types. |
| USBTransMode | transMode | 0 | `0` - Normal, `1` - Compact | Transfer Mode |
| ReturnMode | returnMode | 0 | `0` - Original Car, `1` - Box | |
| BackRecording | backRecording | 0 | 0-1 | |
| FastConnect | fastConnect | 0 | 0-1 | |
| ImprovedFluency | improvedFluency | 0 | 0-1 | Improved Smoothness |
| NaviVolume | naviVolume | 0 | 0-100 | |
| OriginalResolution | originalRes | 0 | 0-1 | |
