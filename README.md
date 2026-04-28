Guitar Synth
===

This [lv2 plugin](https://en.wikipedia.org/wiki/LV2) converts any mono audio signal input (like guitar) to a sawtooth to make it sound like a synth or VCO.
Loudness of the input is preserved. There is a gain control for very quiet inputs.

This is done by checking for zero crossings in the input and generating the sawtooth output algorithmically.
Too low buffer sizes (lower than 64 in my tess) will make the output more sound noisy since there is a simple prediction involved to generate output after a zero crossing until the end of the buffer.

Install
===
Install the pre compiled artifacts from https://github.com/rominator1983/guitarSynth.lv2/releases
- Create a new folder in your lv2 directory (For example ~/.lv2/ or /usr/lib/lv2 or /usr/local/lib/lv2/ would all work in Ubuntu Studio)
- Copy guitarSynth.so, manifest.ttl and guitarSynth.ttl to this new folder.
- Restart your lv2 host and scan for new plugins.

Build on your own
===
-  `./waf configure`
-  `./waf build`
-  `sudo ./waf install`
- It 'might' be neccessary to copy the output to your lv2 directory with: `sudo mv -f /usr/local/lib64/lv2/guitarSynth.lv2 /usr/lib/lv2/` depending on your distro.
- Restart/refresh your lv2 host
