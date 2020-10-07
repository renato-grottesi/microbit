For the Android application, you need to checkout https://github.com/microbit-foundation/microbit-blue and apply the provided microbit-blue.patch before building it.

To successfully pair the micro:bit, use the provided config.json and pair it without a password.
Then set `open` and `pairing_mode` to 1, recompile, reflash and pair again.
On the second attempt, the Android phone will pair it fine.
Now you can use the custom microbit-blue with your micro:bit.
