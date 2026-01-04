WORK IN PROGRESS: Guitar Synth
===

This [lv2 plugin](https://en.wikipedia.org/wiki/LV2) converts any signal input (like guitar) to either sawtooth or square wave.

Install
===
Install the pre compiled artifacts from https://github.com/rominator1983/guitarSynth.lv2/releases
- Create a new folder in your lv2 directory (For example ~/.lv2/ or /usr/lib/lv2 or /usr/local/lib/lv2/ would all work in Ubuntu Studio)
- Copy guitarSynth.so, manifest.ttl and guitarSynth.ttl to this new folder.
- Restart/refresh your lv2 host

Build on your own
===
-  `./waf configure`
-  `./waf build`
-  `sudo ./waf install`
- It 'might' be neccessary to copy the output to your lv2 directory with: `sudo mv -f /usr/local/lib64/lv2/guitarSynth.lv2 /usr/lib/lv2/`
- Restart/refresh your lv2 host
