# blink fork
Blink for prototyping some glitter concepts, primarily custom layout for now.

To use this fork:

Run a usual chromium checkout clone: http://www.chromium.org/developers/how-tos/get-the-code

`$ fetch --nohooks --no-history chromium`

Update main DEPS file to point to this repo:

`
'src/third_party/WebKit':
'https://github.com/GlitterOrg/blink.git',
`

Sync again & build!

`$ gclient sync`
`$ ninja -C out/Debug content_shell`
