This is a utility I'm working on to make configuring i3
and other programs that use X11's sometimes counterintuitive keysym names
a little easier. It was inspired by the time and effort I spent figuring out
that when you press printscreen with alt held down, this generates a Sys_Req
key event as opposed to a Print key event - if there are other similar
peculiarities awaiting me in the keymap, I would rather handle them
programmatically instead of having to manually learn them all, and I would
like to sharpen my C skills up anyway.

