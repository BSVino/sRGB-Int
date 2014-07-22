sRGB <-> Linear conversion for floating point and integer types 
===============================================================

This was a test to see the difference in conversion speed between sRGB and linear when avoiding using floating point. It was originally written as part of stb_resample.

For the integer versions, the linear values are stored such that [0, 1] is remapped to [0, 2^15]. The round trip uchar -> short -> uchar conversion loses a bit of precision, usually no more or less than +-1/255. The Taylor approximation used in stbr__pow_417_times_128 causes it to be off by +-2/255 at points too far from the center, I centered it at 450. This could probably be improved by using two Taylors series with two centers since there's no precision left to add more terms to the current Taylor series.

My results for the difference in performance:

Floating point: 9.021 seconds
Integer: 4.501 seconds

