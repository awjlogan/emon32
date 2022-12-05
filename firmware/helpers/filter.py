# From: https://www.dsprelated.com/showcode/270.php
# The following is a Python/scipy snippet to generate the
# coefficients for a halfband filter.  A halfband filter
# is a filter where the cutoff frequency is Fs/4 and every
# other coeffecient is zero except the cetner tap.
# Note: every other (even except 0) is 0, most of the coefficients
#       will be close to zero, force to zero actual

import numpy
from numpy import log10, abs, pi
import scipy
from scipy import signal
import matplotlib
import matplotlib.pyplot
import matplotlib as mpl

# ~~[Filter Design with Parks-McClellan Remez]~~
N = 26  # Filter order
# Filter symetric around 0.25 (where .5 is pi or Fs/2)
bands = numpy.array([0., .22, .28, .5])
h = signal.remez(N+1, bands, [1,0], [1,1])
h[abs(h) <= 1e-4] = 0.
(w,H) = signal.freqz(h)

# ~~[Filter Design with Windowed freq]~~
b = signal.firwin(N+1, 0.5)
b[abs(h) <= 1e-4] = 0.
(wb, Hb) = signal.freqz(b)

# Dump the coefficients for comparison and verification
print('          remez       firwin')
print('------------------------------------')
for ii in range(N+1):
    print(' tap %2d   %-3.6f    %-3.6f' % (ii, h[ii], b[ii]))
print('\n----- FIXED POINT -----\n')
for ii in range(N+1):
    print(' tap %2d   %5d        %5d' % (ii, int(round(h[ii]*65536.0)), int(round(b[ii]*65536.0))))

## ~~[Plotting]~~
# Note: the pylab functions can be used to create plots,
#       and these might be easier for beginners or more familiar
#       for Matlab users.  pylab is a wrapper around lower-level
#       MPL artist (pyplot) functions.
fig = mpl.pyplot.figure()
ax0 = fig.add_subplot(211)
ax0.stem(numpy.arange(len(h)), h)
ax0.grid(True)
ax0.set_title('Parks-McClellan (remez) Impulse Response')
ax1 = fig.add_subplot(212)
ax1.stem(numpy.arange(len(b)), b)
ax1.set_title('Windowed Frequency Sampling (firwin) Impulse Response')
ax1.grid(True)
fig.savefig('hb_imp.png')
fig.show()

fig = mpl.pyplot.figure()
ax1 = fig.add_subplot(111)
ax1.plot(w, 20*log10(abs(H)))
ax1.plot(w, 20*log10(abs(Hb)))
ax1.legend(['remez', 'firwin'])
bx = bands*2*pi
ax1.axvspan(bx[1], bx[2], facecolor='0.5', alpha=0.33)
ax1.plot(pi/2, -6, 'go')
ax1.axvline(pi/2, color='g', linestyle='--')
ax1.axis([0,pi,-64,3])
ax1.grid('on')
ax1.set_ylabel('Magnitude (dB)')
ax1.set_xlabel('Normalized Frequency (radians)')
ax1.set_title('Half Band Filter Frequency Response')
fig.savefig('hb_rsp.png')
fig.show()
breakpoint()
