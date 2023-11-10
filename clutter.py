import rasterio
from rasterio.warp import calculate_default_transform, reproject, Resampling, transform as warp_transform

import matplotlib.pyplot as plt
import numpy as np
import scipy as sp
import scipy.ndimage

import struct

if __name__ == '__main__':

    SOURCE_ESRI_FILE = "34U_20220101-20230101.tif"
    SOURCE_TF_FILE = "mazowieckie.txt.tf"

    DESTINATION_TIF_FILE = "mazowieckie.tif"
    DESTINATION_CF_FILE = "mazowieckie.tif.cf"
    DESTINATION_CRS = 'EPSG:2180'

    OVERSAMPLE = 2
    SMOOTHING = 3.0

    CLUTTER_HEIGHT_MAP = {
        1: 0,  # Water
        2: 6,  # Trees
        4: 0.1,  # Flooded vegetation
        5: 0.1,  # Crops
        7: 10,  # Built area
        8: 0.1,  # Bare soil
        9: 0.1,  # Snow/ice
        10: 0.1,  # Clouds
        11: 0.1  # Rangeland
    }
    HEIGHT_MAPPING = np.vectorize(CLUTTER_HEIGHT_MAP.get)

    with open(SOURCE_TF_FILE, "rb") as ter:
        y_size = struct.unpack('i', ter.read(4))[0]
        x_size = struct.unpack('i', ter.read(4))[0]
        y = np.fromfile(ter, dtype=np.float64, count=y_size)
        x = np.fromfile(ter, dtype=np.float64, count=x_size)

    dst_shape = (y_size * OVERSAMPLE, x_size * OVERSAMPLE)
    destination = np.zeros(dst_shape, dtype=np.uint8)

    with rasterio.open(SOURCE_ESRI_FILE) as src:
        edges = [(x[0], y[0]), (x[-1], y[0]), (x[-1], y[-1]), (x[0], y[-1])]
        edges = np.array(edges)
        xs, ys = edges.T

        edges_src = np.vstack(warp_transform(
            src_crs=DESTINATION_CRS, dst_crs=src.crs, xs=xs, ys=ys)).T
        xs_src, ys_src = edges_src.T

        left = np.min(xs_src)
        right = np.max(xs_src)
        bottom = np.min(ys_src)
        top = np.max(ys_src)

        transform, width, height = calculate_default_transform(
            src.crs, DESTINATION_CRS, src.width, src.height,
            left=left, right=right, bottom=bottom, top=top,
            dst_width=x_size * OVERSAMPLE,
            dst_height=y_size * OVERSAMPLE)
        
        kwargs = src.meta.copy()
        kwargs.update({
            'crs': DESTINATION_CRS,
            'transform': transform,
            'width': width,
            'height': height,
        })

        with rasterio.open(DESTINATION_TIF_FILE, 'w', **kwargs) as dst:
            reproject(
                source=rasterio.band(src, 1),
                destination=destination,
                src_transform=src.transform,
                src_crs=src.crs,
                dst_transform=transform,
                dst_crs=DESTINATION_CRS,
                resampling=Resampling.nearest,
                num_threads=16)
            dst.write(destination, indexes=1)
            dst.write_colormap(1, src.colormap(1))

    print("TIF file created")

    destination = HEIGHT_MAPPING(destination)

    sigma_y = sigma_x = SMOOTHING
    sigma = [sigma_y, sigma_x]
    destination = sp.ndimage.filters.gaussian_filter(destination, sigma, mode='constant')

    new_y = np.linspace(y[0], y[-1], y_size * OVERSAMPLE, dtype=np.float64)
    new_x = np.linspace(x[0], x[-1], x_size * OVERSAMPLE, dtype=np.float64)
    with open(DESTINATION_CF_FILE, "wb") as dst:
        dst.write(struct.pack('i', y_size * OVERSAMPLE))
        dst.write(struct.pack('i', x_size * OVERSAMPLE))
        new_y.tofile(dst)
        new_x.tofile(dst)
        for iy in range(y_size * OVERSAMPLE):
            row = destination[y_size * OVERSAMPLE - iy - 1]
            row_heights = np.round(row * 100)  # cm
            row_heights.astype(np.uint16).tofile(dst)

    print("CF file created")