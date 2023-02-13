# From: https://www.dsprelated.com/showcode/270.php

# The following is a Python/scipy snippet to generate the
# coefficients for a halfband filter.  A halfband filter
# is a filter where the cutoff frequency is Fs/4 and every
# other coeffecient is zero except the cetner tap.
# Note: every other (even except 0) is 0, most of the coefficients
#       will be close to zero, force to zero actual

import numpy
from numpy import log10, abs, pi
from scipy import signal
import matplotlib
import matplotlib.pyplot
import matplotlib as mpl
import sys

# Filter order
if len(sys.argv) < 2:
    print('[ERROR] Filter order required. Exiting.')
    exit(1)

N = int(sys.argv[1])
fp_format = 32768.0  # Q1.15

# ~~[Filter Design with Parks-McClellan Remez]~~
# Filter symetric around 0.25 (where .5 is pi or Fs/2)
bands = numpy.array([0., .22, .28, .5])
h = signal.remez(N + 1, bands, [1, 0], [1, 1])
h[abs(h) <= 1e-4] = 0.
(w, H) = signal.freqz(h)

# ~~[Filter Design with Windowed freq]~~
b = signal.firwin(N + 1, 0.5)
b[abs(h) <= 1e-4] = 0.
(wb, Hb) = signal.freqz(b)

# (Floating point Dump the coefficients for comparison and verification
# print('          remez       firwin')
# print('------------------------------------')
# for ii in range(N + 1):
#     print(' tap %2d   %-3.6f    %-3.6f' % (ii, h[ii], b[ii]))

# Convert floating point output to fixed point
h_fixed = [int(round(h[idxCoeff] * fp_format)) for idxCoeff in range(N + 1)]
b_fixed = [int(round(b[idxCoeff] * fp_format)) for idxCoeff in range(N + 1)]

print('          remez       firwin')
print('------------------------------------')
for ii in range(N + 1):
    print(' tap %2d   %5d        %5d' % (ii, h_fixed[ii], b_fixed[ii]))
print('')

# Get the number of unique coefficients, excluding 0, and 1st non-zero element
# For emon32, the windowed freq is typically used.
if b_fixed[0] != 0:
    first_coeff = 0
else:
    first_coeff = 1

b_unique = []
for coeff in b_fixed:
    # Do not include 0s
    if coeff == 0:
        continue
    if coeff not in b_unique:
        b_unique.append(coeff)

num_coeffs = len(b_unique)
# Generate the Q1.15 array of coefficients
print('/* Symmetric half band FIR coefficients. 0s not included')
print(f' * first non-zero index is {first_coeff}')
print('*/')
print(f'#define DOWNSAMPLE_TAPS {N+1}u')
print(f'const int16_t firCoeffs[{num_coeffs}] = {{')
for coeff in b_unique:
    print(f'    {coeff}', end='')
    if coeff != b_unique[-1]:
        print(',')
    else:
        print('')
print('};')

# ~~[Plotting]~~
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

fig = mpl.pyplot.figure()
ax1 = fig.add_subplot(111)
ax1.plot(w, 20 * log10(abs(H)))
ax1.plot(w, 20 * log10(abs(Hb)))
ax1.legend(['remez', 'firwin'])
bx = bands * 2 * pi
ax1.axvspan(bx[1], bx[2], facecolor='0.5', alpha=0.33)
ax1.plot(pi / 2, -6, 'go')
ax1.axvline(pi / 2, color='g', linestyle='--')
ax1.axis([0, pi, -64, 3])
ax1.grid('on')
ax1.set_ylabel('Magnitude (dB)')
ax1.set_xlabel('Normalized Frequency (radians)')
ax1.set_title('Half Band Filter Frequency Response')
fig.savefig('hb_rsp.png')

