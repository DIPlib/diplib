import diplib as dip
import numpy as np
import matplotlib.pyplot as plt
import pickle
import scipy.optimize


compute = True


def time_function_call(func):
    timer = dip.testing.Timer()
    func()
    timer.Stop()
    t = timer.GetWall()
    if t > 1:
        return t
    n = min(20, int(np.ceil(1 / t)))
    timer.Reset()
    for _ in range(n):
        func()
    timer.Stop()
    return timer.GetWall() / n


### Compute timings ###

if compute:
    img_sizes = [(256,256), (512,512), (768,768), (1024,1024), (1280,1280), (2048,2048), (1024,768), (2048,1028), (64,64,64), (96,96,64), (128,128,64), (256,64,32)]
    kernel_sizes = [3, 7, 13, 19, 25, 31, 37, 43, 49, 61, 73, 85, 97, 109, 121]
    time_fft = np.zeros((len(img_sizes), len(kernel_sizes))) + np.nan
    time_sep = np.zeros((len(img_sizes), len(kernel_sizes))) + np.nan
    time_dir = np.zeros((len(img_sizes), len(kernel_sizes))) + np.nan
    for jj, imsz in enumerate(img_sizes):
        print(f"Image size = {imsz}")
        img = np.random.rand(*imsz)
        for ii, ksz in enumerate(kernel_sizes):
            if ksz > np.min(imsz):
                continue  # The FFT method will not do this case
            kernel = np.ones((ksz,) * len(imsz))
            kernel = dip.Image(kernel / np.sum(kernel), None)
            time_fft[jj, ii] = time_function_call(lambda: dip.Convolution(img, kernel, "fourier"))
            time_sep[jj, ii] = time_function_call(lambda: dip.Convolution(img, kernel, "separable"))
            if ksz > 25:
                continue  # Let's not do the direct method for these sizes, it'll be very expensive
            time_dir[jj, ii] = time_function_call(lambda: dip.Convolution(img, kernel, "direct"))
    with open("convolution_timing_test.pickle", "wb") as f:
        pickle.dump((img_sizes, kernel_sizes, time_fft, time_sep, time_dir), f)
else:
    # Load previously computed timings
    with open("convolution_timing_test.pickle", "rb") as f:
        img_sizes, kernel_sizes, time_fft, time_sep, time_dir = pickle.load(f)


### Fit the models, and make a plot to see how good they are ###

kernel_sizes, img_sizes = np.meshgrid(kernel_sizes, np.array(img_sizes, dtype=object))
img_sizes = img_sizes.ravel()
kernel_sizes = kernel_sizes.ravel()
time_fft = time_fft.ravel()
time_sep = time_sep.ravel()
time_dir = time_dir.ravel()

# print(np.stack((kernel_sizes, time_fft, time_sep, time_dir), axis=-1))

img_numel = np.array([np.prod(imsz) for imsz in img_sizes])
img_ndims = np.array([len(imsz) for imsz in img_sizes])

# model_fft: n = number of pixels of boundary expanded image
#            t = p0 * n * np.log2(n) + p1 * n + p2
# p0 turns out to be tiny, and we'll ignore it:
#            t = p0 * n + p1
data_fft = np.array([np.prod(np.add(imsz, ksz)) for imsz, ksz in zip(img_sizes, kernel_sizes)])
model_fft = lambda n, p0, p1: p0 * n + p1
index = np.isfinite(time_fft) & (time_fft < 0.25)
time_fft = time_fft[index]
data_fft = data_fft[index]

# model_sep: n = number of pixels
#            d = number of dimensions
#            t = p0 * n * ksz * d + p1 * n + p2 * ksz * d + p3
# p1 and p3 turn out to be tiny, and we'll ignore them:
#            t = p0 * n * ksz * d + p1 * ksz * d
data_sep = np.stack((img_numel, kernel_sizes * img_ndims), axis=-1)
model_sep = lambda n, p0, p1: p0 * n[0,:] * n[1,:] + p1 * n[1,:]
index = np.isfinite(time_sep) & (time_sep < 0.25)
time_sep = time_sep[index]
data_sep = data_sep[index].transpose()

# model_dir: n = number of pixels
#            d = number of dimensions
#            t = p0 * n * ksz ** d + p1 * n + p2 * ksz ** d + p3
# p1 and p3 turn out to be tiny, and we'll ignore them:
#            t = p0 * n * ksz ** d + p1 * ksz ** d
data_dir = np.stack((img_numel, kernel_sizes ** img_ndims), axis=-1)
model_dir = lambda n, p0, p1: p0 * n[0,:] * n[1,:] + p1 * n[1,:]
index = np.isfinite(time_dir) & (time_dir < 0.25)
time_dir = time_dir[index]
data_dir = data_dir[index].transpose()

params_fft, _ = scipy.optimize.curve_fit(model_fft, data_fft, time_fft, p0=(1,1), bounds=(0, np.inf))
print(f"Fourier parameters: p0 = {params_fft[0]:.3e}, p1 = {params_fft[1]:.3e}")
estimate_fft = model_fft(data_fft, *params_fft)

params_sep, _ = scipy.optimize.curve_fit(model_sep, data_sep, time_sep, p0=(1,1), bounds=(0, np.inf))
print(f"Separable parameters: p0 = {params_sep[0]:.3e}, p1 = {params_sep[1]:.3e}")
estimate_sep = model_sep(data_sep, *params_sep)

params_dir, _ = scipy.optimize.curve_fit(model_dir, data_dir, time_dir, p0=(1,1), bounds=(0, np.inf))
print(f"Direct parameters: p0 = {params_dir[0]:.3e}, p1 = {params_dir[1]:.3e}")
estimate_dir = model_dir(data_dir, *params_dir)

plt.scatter(time_fft, estimate_fft, label="Fourier", s=1, alpha=0.5)
plt.scatter(time_sep, estimate_sep, label="separable", s=1, alpha=0.5)
plt.scatter(time_dir, estimate_dir, label="direct", s=1, alpha=0.5)
plt.xscale("log")
plt.yscale("log")
plt.legend()
plt.show()

# Apple Silicon M1 machine:
#   Fourier parameters: p0 = 1.635e-08, p1 = 8.781e-04
#   Separable parameters: p0 = 1.434e-10, p1 = 4.987e-06
#   Direct parameters: p0 = 1.806e-10, p1 = 1.206e-05
